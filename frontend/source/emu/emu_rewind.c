#include <stdlib.h>
#include <string.h>
#include <libretro.h>
#include <psp2/kernel/processmgr.h>
#include "emu_rewind.h"
#include "emu_game.h"
#include "emu_audio.h"
#include "utils.h"

// 每 0.1 秒记录一次
#define NEXT_STATE_PERIOD 100000
// 如果差异的大小超过 THRESHOLD_RATE，则保存全部
#define THRESHOLD_RATE 0.1
// 最小差异单位
#define DIFF_STEP 0x10

enum BlockType
{
    BLOCK_FULL,
    BLOCK_DIFF
};

typedef struct
{
    int type;
    size_t total_size;
    void *prev;
    void *next;
} BlockHeader;

// 用于保存完整的状态信息
typedef struct FullBlockStruct
{
    BlockHeader header;
    struct FullBlockStruct *next_full_block;
    uint8_t buf[];
} FullBlock;

// 用于保存差异的状态信息
typedef struct
{
    uint32_t offset;
    uint32_t size;
} DiffArea;

typedef struct
{
    BlockHeader header;
    FullBlock *full_block;
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
    bh->next = (void *)bh + bh->total_size;
    if (bh->next > rs.tail - rs.state_size - 0x20)
        bh->next = rs.data;

    if (bh->next >= (void *)rs.header && bh->next < (void *)rs.header + rs.header->header.total_size)
    {
        // next 落入 header 的区域
        if (rs.header->next_full_block)
            rs.header = rs.header->next_full_block; // 指向下一个 FullBlock
        else
            bh->next = NULL; // 只有一个 FullBlock
    }
    return bh->next;
}

static void SaveFullState(FullBlock *fb)
{
    fb->header.type = BLOCK_FULL;
    fb->header.total_size = sizeof(FullBlock) + rs.state_size;
    fb->header.prev = 0;
    fb->header.next = 0;
    fb->next_full_block = NULL;
    retro_serialize(&(fb->buf), rs.state_size);
}

// 仅把需要复制的差异信息(地址, 大小)写入 areas，统计 total_size
static void PreSaveDiffState(DiffBlock *db, FullBlock *fb)
{
    db->header.type = BLOCK_DIFF;
    db->header.total_size = sizeof(DiffBlock);
    db->header.prev = 0;
    db->header.next = 0;
    db->full_block = fb;
    db->num = 0;
    retro_serialize(rs.tmp_state, rs.state_size);

    uint8_t *old = fb->buf;
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
                db->areas[db->num].size = offset - db->areas[db->num].offset;
                db->header.total_size += db->areas[db->num].size;
                db->num++;
            }
        }
        else
        {
            if (last_state == 0)
            {
                last_state = 1;
                db->areas[db->num].offset = offset;
            }
        }
    }

    uint32_t tail_size = rs.state_size % DIFF_STEP;
    offset = rs.state_size - tail_size;
    if (tail_size > 0 && memcmp(old + offset, new + offset, tail_size) != 0)
    {
        if (last_state == 0)
        {
            db->areas[db->num].offset = offset;
            db->areas[db->num].size = tail_size;
        }
        else
        {
            db->areas[db->num].size = rs.state_size - db->areas[db->num].offset;
        }

        db->header.total_size += db->areas[db->num].size;
        db->num++;
    }
    else if (last_state == 1)
    {
        db->areas[db->num].size = offset - db->areas[db->num].offset;
        db->header.total_size += db->areas[db->num].size;
        db->num++;
    }

    db->header.total_size += db->num * sizeof(DiffArea);
}

static void SaveDiffState(DiffBlock *db)
{
    uint8_t *buf = (uint8_t *)db + sizeof(DiffBlock) + db->num * sizeof(DiffArea);
    DiffArea *area = db->areas;
    for (int i = 0; i < db->num; i++)
    {
        memcpy(buf, rs.tmp_state + area->offset, area->size);
        buf += area->size;
        area++;
    }
}

static const void *GetState()
{
    BlockHeader *bh = (BlockHeader *)rs.current;

    if (bh->type == BLOCK_FULL)
        return ((FullBlock *)bh)->buf;

    DiffBlock *db = (DiffBlock *)bh;
    uint8_t *buf = (uint8_t *)db + sizeof(DiffBlock) + db->num * sizeof(DiffArea);
    DiffArea *area = db->areas;

    memcpy(rs.tmp_state, db->full_block->buf, rs.state_size);
    for (int i = 0; i < db->num; i++)
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
                    ((FullBlock *)current)->next_full_block = (FullBlock *)next;
                else
                    ((DiffBlock *)current)->full_block->next_full_block = (FullBlock *)next;
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