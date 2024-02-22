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

typedef struct FullBlock FullBlock;

#define BLOCK_HEADER                                          \
    int type;              /* BLOCK_FULL 或 BLOCK_DIFF*/     \
    size_t total_size;     /* 整个 block 完整的大小 */ \
    void *prev;            /* 指向上一个 block */        \
    FullBlock *full_block; /* type == BLOCK_FULL 时指向下一个 FullBlock, BLOCK_DIFF 时指向基础的 FullBlock */

typedef struct
{
    BLOCK_HEADER
} BlockHeader;

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

typedef struct
{
    BLOCK_HEADER
    uint32_t num;
    DiffArea areas[];
    // uin8_t *diff_buf;
} DiffBlock;

// 缓存的全局状态
typedef struct
{
    void *data;            // 分配得到的内存地址
    void *current;         // 指向当前的状态
    FullBlock *header;     // 指向最早的状态
    void *tail;            // 指向 data 的尾部
    size_t state_size;     // 完整状态的实际大小
    size_t threshold_size; // state_size * THRESHOLD_RATE
    uint64_t next_time;    // 下一次记录的时间
    void *tmp_state;       // 用于保存差异状态
} RewindState;

RewindState rs = {0};
int rewind_key_pressed = 0;
int in_rewinding = 0;

static void *SetNextBlock(BlockHeader *bh)
{
    void *next = (uint8_t *)bh + bh->total_size;
    if (next > rs.tail - rs.state_size - 0x20)
        next = rs.data;

    void *start1 = next;
    void *end1 = (uint8_t *)next + sizeof(FullBlock) + rs.state_size;
    void *start2 = rs.header;
    void *end2 = (uint8_t *)rs.header + rs.header->total_size;

    if (!(end1 < start2 || start1 >= end2))
    {
        // next 落入 header 的区域
        if (rs.header->full_block)
            rs.header = rs.header->full_block; // 指向下一个 FullBlock
        else
            next = NULL; // 只有一个 FullBlock
    }
    return next;
}

static void SaveFullState(FullBlock *fullb)
{
    fullb->type = BLOCK_FULL;
    fullb->total_size = sizeof(FullBlock) + rs.state_size;
    fullb->prev = 0;
    fullb->full_block = NULL;
    retro_serialize(fullb->buf, rs.state_size);
}

// 仅把需要复制的差异信息(地址, 大小)写入 areas，统计 total_size
static void PreSaveDiffState(DiffBlock *diffb, FullBlock *fullb)
{
    diffb->type = BLOCK_DIFF;
    diffb->total_size = sizeof(DiffBlock);
    diffb->prev = 0;
    diffb->full_block = fullb;
    diffb->num = 0;
    retro_serialize(rs.tmp_state, rs.state_size);

    uint8_t *old = fullb->buf;
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
                diffb->areas[diffb->num].size = offset - diffb->areas[diffb->num].offset;
                diffb->total_size += diffb->areas[diffb->num].size;
                diffb->num++;
            }
        }
        else
        {
            if (last_state == 0)
            {
                last_state = 1;
                diffb->areas[diffb->num].offset = offset;
            }
        }
    }

    uint32_t tail_size = rs.state_size % DIFF_STEP;
    offset = rs.state_size - tail_size;
    if (tail_size > 0 && memcmp(old + offset, new + offset, tail_size) != 0)
    {
        if (last_state == 0)
        {
            diffb->areas[diffb->num].offset = offset;
            diffb->areas[diffb->num].size = tail_size;
        }
        else
        {
            diffb->areas[diffb->num].size = rs.state_size - diffb->areas[diffb->num].offset;
        }

        diffb->total_size += diffb->areas[diffb->num].size;
        diffb->num++;
    }
    else if (last_state == 1)
    {
        diffb->areas[diffb->num].size = offset - diffb->areas[diffb->num].offset;
        diffb->total_size += diffb->areas[diffb->num].size;
        diffb->num++;
    }

    diffb->total_size += diffb->num * sizeof(DiffArea);
}

static void SaveDiffState(DiffBlock *diffb)
{
    uint8_t *buf = (uint8_t *)diffb + sizeof(DiffBlock) + diffb->num * sizeof(DiffArea);
    DiffArea *area = diffb->areas;
    for (int i = 0; i < diffb->num; i++)
    {
        memcpy(buf, rs.tmp_state + area->offset, area->size);
        buf += area->size;
        area++;
    }
}

static const void *GetState()
{
    BlockHeader *header = (BlockHeader *)rs.current;

    if (header->type == BLOCK_FULL)
        return ((FullBlock *)header)->buf;

    DiffBlock *diffb = (DiffBlock *)header;
    uint8_t *buf = (uint8_t *)diffb + sizeof(DiffBlock) + diffb->num * sizeof(DiffArea);
    DiffArea *area = diffb->areas;

    memcpy(rs.tmp_state, diffb->full_block->buf, rs.state_size);
    for (int i = 0; i < diffb->num; i++)
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
    if (rs.current && rs.current != rs.header)
    {
        if (Emu_GetCurrentRunSpeed() > 0.1)
        {
            Emu_SetRunSpeed(0.1);
            Emu_PauseAudio();
            in_rewinding = 1;
        }
        const void *state = GetState();
        retro_unserialize(state, rs.state_size);
        rs.current = ((BlockHeader *)rs.current)->prev;
        Emu_CleanAudioSound();
    }

    rewind_key_pressed = 0;
}

static void SaveState()
{
    if (rs.current == NULL)
    {
        rs.current = rs.data;
        rs.header = rs.data;
        SaveFullState((FullBlock *)rs.current);
    }
    else
    {
        BlockHeader *current = (BlockHeader *)rs.current;
        BlockHeader *next = SetNextBlock(current);
        if (next)
        {
            FullBlock *fb = current->type == BLOCK_FULL ? (FullBlock *)current : ((DiffBlock *)current)->full_block;
            PreSaveDiffState((DiffBlock *)next, fb);
            if (next->total_size > rs.threshold_size)
            {
                SaveFullState((FullBlock *)next);
                if (current->type == BLOCK_FULL)
                    current->full_block = (FullBlock *)next;
                else
                    current->full_block->full_block = (FullBlock *)next;
            }
            else
            {
                SaveDiffState((DiffBlock *)next);
            }

            rs.current = next;
            next->prev = current;
        }
    }

    if (Emu_GetCurrentRunSpeed() <= 0.15)
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