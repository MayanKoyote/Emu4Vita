#include <stdlib.h>
#include <string.h>
#include <libretro.h>
#include <psp2/kernel/processmgr.h>
#include "emu_rewind.h"
#include "emu_game.h"
#include "emu_audio.h"
#include "utils.h"

// 每 0.05 秒记录一次
#define NEXT_STATE_PERIOD 50000
// 如果差异的大小超过 THRESHOLD_RATE，则保存全部
#define THRESHOLD_RATE 0.1
// 最小差异单位
#define DIFF_STEP 0x10

enum BlockType
{
    BLOCK_FULL,
    BLOCK_DIFF
};

typedef struct BlockBase BlockBase;
typedef struct FullBlock FullBlock;
typedef struct DiffBlock DiffBlock;

#define BLOCK_HEADER                                      \
    int type;              /* BLOCK_FULL 或 BLOCK_DIFF*/ \
    BlockBase *next;       /* 指向下一个 block */    \
    BlockBase *prev;       /* 指向上一个 block */    \
    FullBlock *full_block; /* type == BLOCK_FULL 时指向下一个 FullBlock, BLOCK_DIFF 时指向基础的 FullBlock */

struct BlockBase
{
    BLOCK_HEADER
};

// 用于保存完整的状态信息
struct FullBlock
{
    BLOCK_HEADER
    uint8_t buf[];
};

// 用于保存差异的状态信息
typedef struct
{
    uint32_t offset;
    uint32_t size;
} DiffArea;

struct DiffBlock
{
    BLOCK_HEADER
    uint32_t num;
    DiffArea areas[];
    // uin8_t *diff_buf;
};

// 缓存的全局状态
typedef struct
{
    uint8_t *data;         // 分配得到的内存地址
    BlockBase *current;    // 指向当前的状态
    FullBlock *header;     // 指向最早的状态
    uint8_t *tail;         // 指向 data 的尾部
    size_t state_size;     // 完整状态的实际大小
    size_t threshold_size; // state_size * THRESHOLD_RATE
    size_t buf_size;       // 缓存大小，用于判断是否新开一个完全状态
    uint64_t next_time;    // 下一次记录的时间
    uint8_t *tmp_state;    // 用于保存差异状态
} RewindState;

RewindState rs = {0};
int rewind_key_pressed = 0;
int in_rewinding = 0;

static void *GetNextBlock(BlockBase *block)
{
    uint8_t *next = (uint8_t *)(block->next);
    if (next > rs.tail - sizeof(FullBlock) - rs.state_size)
        next = rs.data;

    if (next < (uint8_t *)rs.header->next && next + sizeof(FullBlock) + rs.state_size > (uint8_t *)rs.header)
    {
        // next 块落入 header 的区域
        AppLog("SetNextBlock %08x %08x %08x\n", next, rs.header, rs.header->full_block);
        if (rs.header->full_block)
            rs.header = rs.header->full_block; // rs.header 指向下一个 FullBlock
        else
            next = NULL; // 只有一个 FullBlock
    }
    return next;
}

static void SaveFullState(FullBlock *full, BlockBase *prev)
{
    full->type = BLOCK_FULL;
    full->next = (BlockBase *)(((uint8_t *)full) + sizeof(FullBlock) + rs.state_size);
    full->prev = prev;
    full->full_block = NULL;
    retro_serialize(full->buf, rs.state_size);
    AppLog("SaveFullState %08x %08x\n", full, full->next);

    if (prev)
    {
        if (prev->type == BLOCK_FULL)
            prev->full_block = full;
        else
            prev->full_block->full_block = full;
    }
}

// 仅把需要复制的差异信息(地址, 大小)写入 areas，统计 total_size
static void PreSaveDiffState(DiffBlock *diff, FullBlock *full)
{
    diff->type = BLOCK_DIFF;
    size_t total_size = sizeof(DiffBlock);
    diff->prev = NULL;
    diff->full_block = full;
    diff->num = 0;
    retro_serialize(rs.tmp_state, rs.state_size);

    uint8_t *old = full->buf;
    uint8_t *new = rs.tmp_state;
    int last_state = 0; // 相同: 0, 不同: 1
    int offset = 0;
    for (; offset < rs.state_size; offset += DIFF_STEP)
    {
        if (memcmp(old + offset, new + offset, DIFF_STEP) == 0)
        {
            if (last_state == 1)
            {
                last_state = 0;
                diff->areas[diff->num].size = offset - diff->areas[diff->num].offset;
                total_size += diff->areas[diff->num].size;
                diff->num++;
            }
        }
        else
        {
            if (last_state == 0)
            {
                last_state = 1;
                diff->areas[diff->num].offset = offset;
            }
        }
    }

    uint32_t tail_size = rs.state_size % DIFF_STEP;
    offset = rs.state_size - tail_size;
    if (tail_size > 0 && memcmp(old + offset, new + offset, tail_size) != 0)
    {
        if (last_state == 0)
        {
            diff->areas[diff->num].offset = offset;
            diff->areas[diff->num].size = tail_size;
        }
        else
        {
            diff->areas[diff->num].size = rs.state_size - diff->areas[diff->num].offset;
        }

        total_size += diff->areas[diff->num].size;
        diff->num++;
    }
    else if (last_state == 1)
    {
        diff->areas[diff->num].size = offset - diff->areas[diff->num].offset;
        total_size += diff->areas[diff->num].size;
        diff->num++;
    }

    total_size += diff->num * sizeof(DiffArea);

    diff->next = (BlockBase *)((uint8_t *)diff + total_size);
}

static void SaveDiffState(DiffBlock *diff, BlockBase *prev)
{
    diff->prev = prev;
    uint8_t *buf = (uint8_t *)diff + sizeof(DiffBlock) + diff->num * sizeof(DiffArea);
    DiffArea *area = diff->areas;
    for (int i = 0; i < diff->num; i++)
    {
        memcpy(buf, rs.tmp_state + area->offset, area->size);
        buf += area->size;
        area++;
    }
}

static const void *GetState(BlockBase *block)
{
    if (block->type == BLOCK_FULL)
        return ((FullBlock *)block)->buf;

    DiffBlock *diff = (DiffBlock *)block;
    uint8_t *buf = (uint8_t *)diff + sizeof(DiffBlock) + diff->num * sizeof(DiffArea);
    DiffArea *area = diff->areas;

    memcpy(rs.tmp_state, diff->full_block->buf, rs.state_size);
    for (int i = 0; i < diff->num; i++)
    {
        memcpy(rs.tmp_state + area->offset, buf, area->size);
        buf += area->size;
        area++;
    }

    return rs.tmp_state;
}

void Emu_InitRewind(size_t buffer_size)
{
    Emu_DeinitRewind();

    rs.state_size = retro_serialize_size();
    rs.threshold_size = rs.state_size * THRESHOLD_RATE;
    rs.buf_size = buffer_size;
    rs.data = malloc(buffer_size);
    rs.header = NULL;
    rs.current = NULL;
    rs.tail = rs.data + buffer_size;
    rs.tmp_state = malloc(rs.state_size);
    rs.next_time = 0;

    AppLog("[REWIND] Rewind_Init: buf size: 0x%08x, state_size: 0x%08x data address: 0x%08x\n", buffer_size, rs.state_size, rs.data);
}

void Emu_DeinitRewind()
{
    if (rs.data)
        free(rs.data);
    if (rs.tmp_state)
        free(rs.tmp_state);
    memset(&rs, 0, sizeof(rs));
}

static void Rewind()
{
    if (rs.current && rs.current != (BlockBase *)rs.header)
    {
        if (in_rewinding == 0 && Emu_GetCurrentRunSpeed() > 0.1)
        {
            Emu_SetRunSpeed(0.1);
            Emu_PauseAudio();
            in_rewinding = 1;
        }

        retro_unserialize(GetState(rs.current), rs.state_size);
        rs.current = rs.current->prev;
        Emu_CleanAudioSound();
    }

    rewind_key_pressed = 0;
}

static size_t GetDistance()
{
    if ((size_t)rs.current >= (size_t)rs.header)
        return (size_t)rs.current - (size_t)rs.header;
    else
        return rs.buf_size + (size_t)rs.header - (size_t)rs.current;
}

static void SaveState()
{
    if (rs.current == NULL)
    {
    CREATE_NEW_STATE:
        rs.current = (BlockBase *)rs.data;
        rs.header = (FullBlock *)rs.data;
        SaveFullState(rs.header, NULL);
    }
    else
    {
        BlockBase *current = rs.current;
        BlockBase *next = GetNextBlock(current);

        if (next)
        {
            if ((!rs.header->full_block) && GetDistance() > rs.buf_size / 2)
                SaveFullState((FullBlock *)next, current);
            else
            {
                FullBlock *fb = current->type == BLOCK_FULL ? (FullBlock *)current : ((DiffBlock *)current)->full_block;
                PreSaveDiffState((DiffBlock *)next, fb);
                if ((size_t)(next->next) - (size_t)next > rs.threshold_size)
                    SaveFullState((FullBlock *)next, current);
                else
                    SaveDiffState((DiffBlock *)next, current);
            }

            rs.current = next;
        }
        else
        {
            goto CREATE_NEW_STATE;
        }
    }

    if (in_rewinding && Emu_GetCurrentRunSpeed() <= 0.15)
    {
        Emu_SetRunSpeed(1.f);
        Emu_ResumeAudio();
        in_rewinding = 0;
    }
}

void Emu_RewindCheck()
{
    if ((!Emu_IsGameLoaded()) || rs.data == NULL || sceKernelGetProcessTimeWide() < rs.next_time)
        return;

    if (rewind_key_pressed)
        Rewind();
    else
        SaveState();

    rs.next_time = sceKernelGetProcessTimeWide() + NEXT_STATE_PERIOD;
}