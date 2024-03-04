#include <stdlib.h>
#include <string.h>
#include <libretro.h>
#include <psp2/kernel/processmgr.h>
#include "emu_rewind.h"
#include "emu_game.h"
#include "emu_audio.h"
#include "emu_cheat.h"
#include "emu_video.h"
#include "utils.h"

// 每 0.05 秒记录一次
#define NEXT_STATE_PERIOD 50000
// 如果差异的大小超过 THRESHOLD_RATE，则保存全部
#define THRESHOLD_RATE 0.1
// 最小差异单位
#define DIFF_STEP 0x10
// "REWD"
#define REWIND_BLOCK_MAGIC 0x44574552
// RewindBlock 的数量
#define BLOCK_SIZE 0x400
// 最小的 buf size 的倍率
#define MIN_STATE_RATE 5

enum BlockType
{
    BLOCK_FULL,
    BLOCK_DIFF
};

typedef struct RewindBlock RewindBlock;

struct RewindBlock
{
    uint32_t type;  // BLOCK_FULL 或 BLOCK_DIFF
    uint32_t index; // 流水号
    uint8_t *offset;
    uint32_t size;
};

#define REWIND_BUF_HEADER \
    uint32_t magic;       \
    uint32_t index;

typedef struct
{
    REWIND_BUF_HEADER
} RewindBufHeader;

typedef struct
{
    REWIND_BUF_HEADER
    uint8_t buf[];
} RewindFullBuf;

typedef struct
{
    uint32_t offset;
    uint32_t size;
} DiffArea;

typedef struct
{
    REWIND_BUF_HEADER
    RewindBlock *full_block;
    uint32_t num;
    DiffArea areas[];
    // 接 uint8_t *diff_buf;
} RewindDiffBuf;

// 缓存的全局状态
typedef struct
{
    RewindBlock *blocks;          // RewindBlock[BLOCK_SIZE]
    RewindBlock *current_block;   // 指向当前的状态
    RewindBlock *last_full_block; // 最新的完整状态
    uint8_t *buf;                 // 分配得到的内存地址
    uint8_t *tmp_buf;             // 用于保存差异状态
    size_t buf_size;              // buf 缓存的大小
    size_t threshold_size;        // state_size * THRESHOLD_RATE, 如果差异的数据大于 threshold_size, 则保存完全的状态
    size_t state_size;            // 完整状态的实际大小
    uint64_t next_time;           // 下一次记录的时间
    uint32_t count;               // 用于设置 RewindBlock 中的 index
} RewindState;

typedef struct
{
    SceUID id;
    SceUID semaphore;
    SceKernelLwMutexWork mutex;
} RewindSavingThread;

RewindState rs = {0};
RewindSavingThread rs_thread = {0};
int rewind_key_pressed = 0;
int in_rewinding = 0;

static RewindBlock *GetNextBlock()
{
    RewindBlock *block = rs.current_block + 1;
    if (block - rs.blocks >= BLOCK_SIZE)
        block = rs.blocks;
    return block;
}

static RewindBlock *GetPreviousBlock()
{
    RewindBlock *block = rs.current_block - 1;
    if (block < rs.blocks)
        block = rs.blocks + BLOCK_SIZE - 1;
    return block;
}

static uint8_t *GetNextBuf()
{
    uint8_t *buf = rs.current_block->offset + rs.current_block->size;
    if (buf + rs.state_size + sizeof(RewindFullBuf) >= rs.buf + rs.buf_size)
        buf = rs.buf;
    return buf;
}

static int IsValidBlock(const RewindBlock *block)
{
    if (!block)
        return 0;

    RewindBufHeader *header = (RewindBufHeader *)block->offset;
    return header && header->magic == REWIND_BLOCK_MAGIC && header->index == block->index;
}

static int SaveFullState(RewindBlock *block, uint8_t *buf_offset, uint8_t *state)
{
    block->type = BLOCK_FULL;
    block->index = rs.count++;
    block->offset = buf_offset;
    block->size = sizeof(RewindFullBuf) + rs.state_size;
    rs.last_full_block = block;
    RewindFullBuf *full = (RewindFullBuf *)buf_offset;

    full->magic = REWIND_BLOCK_MAGIC;
    full->index = block->index;
    if (state)
    {
        memcpy(full->buf, state, rs.state_size);
        return 1;
    }
    else
        return retro_serialize(full->buf, rs.state_size);
    // AppLog("SaveFullState %08x %08x\n", block, buf_offset);
}

// 仅把需要复制的差异信息(地址, 大小)写入 areas，统计 total_size
static int PreSaveDiffState(RewindBlock *block, uint8_t *buf_offset, RewindBlock *full_block)
{
    block->type = BLOCK_DIFF;
    block->index = rs.count++;
    block->offset = buf_offset;
    block->size = sizeof(RewindDiffBuf);

    RewindDiffBuf *diff = (RewindDiffBuf *)buf_offset;
    diff->magic = REWIND_BLOCK_MAGIC;
    diff->index = block->index;
    diff->full_block = full_block;
    diff->num = 0;
    // if (!retro_serialize(rs.tmp_buf, rs.state_size))
    //     return 0;

    uint8_t *old = ((RewindFullBuf *)full_block->offset)->buf;
    uint8_t *new = rs.tmp_buf;
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
                block->size += diff->areas[diff->num].size;
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

        block->size += diff->areas[diff->num].size;
        diff->num++;
    }
    else if (last_state == 1)
    {
        diff->areas[diff->num].size = offset - diff->areas[diff->num].offset;
        block->size += diff->areas[diff->num].size;
        diff->num++;
    }

    block->size += diff->num * sizeof(DiffArea);
    return 1;
}

static void SaveDiffState(RewindBlock *block)
{
    RewindDiffBuf *diff = (RewindDiffBuf *)block->offset;
    uint8_t *buf = (uint8_t *)diff + sizeof(RewindDiffBuf) + diff->num * sizeof(DiffArea);
    DiffArea *area = diff->areas;
    for (int i = 0; i < diff->num; i++)
    {
        memcpy(buf, rs.tmp_buf + area->offset, area->size);
        buf += area->size;
        area++;
    }
    // AppLog("SaveDiffState %08x %08x %d\n", block, diff, diff->num);
}

static void *GetState(RewindBlock *block)
{
    if (!IsValidBlock(block))
        return NULL;

    if (block->type == BLOCK_FULL)
        return ((RewindFullBuf *)block->offset)->buf;

    RewindDiffBuf *diff = (RewindDiffBuf *)block->offset;
    if (!IsValidBlock(diff->full_block))
        return NULL;

    uint8_t *buf = block->offset + sizeof(RewindDiffBuf) + diff->num * sizeof(DiffArea);
    DiffArea *area = diff->areas;

    memcpy(rs.tmp_buf, ((RewindFullBuf *)diff->full_block->offset)->buf, rs.state_size);
    for (int i = 0; i < diff->num; i++)
    {
        memcpy(rs.tmp_buf + area->offset, buf, area->size);
        buf += area->size;
        area++;
    }

    return rs.tmp_buf;
}

static void Rewind()
{
    sceKernelLockLwMutex(&rs_thread.mutex, 1, NULL);

    if (in_rewinding == 0)
    {
        Emu_PauseCheat();
        Emu_PauseAudio();
        in_rewinding = 1;
    }

    RewindBlock *prev = GetPreviousBlock();
    // AppLog("%08x %08x\n", rs.current_block, prev);
    if (IsValidBlock(prev))
    {
        void *state = GetState(rs.current_block);
        if (state)
        {
            retro_unserialize(state, rs.state_size);
            rs.current_block = prev;
            Emu_CleanAudioSound();
            if (Emu_IsVideoPaused())
                Emu_ResumeVideo();
        }
    }
    // AppLog("Rewind End\n");
    rewind_key_pressed = 0;

    sceKernelUnlockLwMutex(&rs_thread.mutex, 1);
}

static size_t GetBufDistance(size_t first, size_t second)
{
    if (second >= first)
        return second - first;
    else
        return rs.buf_size + first - second;
}

static void SaveState()
{
    if (rs.current_block == NULL)
    {
        // 进游戏后首次保存
        if (SaveFullState(rs.blocks, rs.buf, NULL))
            rs.current_block = rs.blocks;
    }
    else if (sceKernelTryLockLwMutex(&rs_thread.mutex, 1) == SCE_OK) // 确保 SavingThreadFunc 没有在运行中
    {
        sceKernelUnlockLwMutex(&rs_thread.mutex, 1);
        if (retro_serialize(rs.tmp_buf, rs.state_size))
        {
            sceKernelSignalSema(rs_thread.semaphore, 1); // 激活 SavingThreadFunc
        }
    }
}

static int SavingThreadFunc()
{
    while (sceKernelWaitSema(rs_thread.semaphore, 1, NULL) == SCE_OK && rs.buf != NULL)
    {
        sceKernelLockLwMutex(&rs_thread.mutex, 1, NULL);

        RewindBlock *current = rs.current_block;
        RewindBlock *next = GetNextBlock();
        uint8_t *next_buf = GetNextBuf();
        int result = 0;
        if (GetBufDistance((size_t)rs.last_full_block->offset, (size_t)next_buf) > rs.buf_size / 2)
            result = SaveFullState(next, next_buf, rs.tmp_buf);
        else
        {
            RewindBlock *full_block = current->type == BLOCK_FULL ? current : ((RewindDiffBuf *)(current->offset))->full_block;
            result = PreSaveDiffState(next, next_buf, full_block);
            if (result)
            {
                if (next->size > rs.threshold_size)
                    result = SaveFullState(next, next_buf, rs.tmp_buf);
                else
                    SaveDiffState(next);
            }
        }

        if (result)
            rs.current_block = next;

        sceKernelUnlockLwMutex(&rs_thread.mutex, 1);
    }

    AppLog("SavingThreadFunc end\n");
    return 0;
}

void Emu_InitRewind(size_t buffer_size)
{
    AppLog("[REWIND] rewind init...\n");
    Emu_DeinitRewind();

    rs.state_size = retro_serialize_size();
    if (buffer_size < rs.state_size * MIN_STATE_RATE)
        return;

    rs.buf_size = buffer_size - sizeof(RewindBlock) * BLOCK_SIZE - rs.state_size;
    rs.threshold_size = rs.state_size * THRESHOLD_RATE;

    rs.blocks = malloc(sizeof(RewindBlock) * BLOCK_SIZE);
    memset(rs.blocks, 0, sizeof(RewindBlock) * BLOCK_SIZE);

    rs.buf = malloc(rs.buf_size);
    memset(rs.buf, 0, sizeof(rs.buf_size));

    rs.tmp_buf = malloc(rs.state_size);

    sceKernelCreateLwMutex(&rs_thread.mutex, "saving_mutex", 0, 0, NULL);
    rs_thread.semaphore = sceKernelCreateSema("saving_emaphore", 0, 0, 1, NULL);
    rs_thread.id = sceKernelCreateThread("saving_thread", SavingThreadFunc, 191, 0x10000, 0, 0, NULL);
    sceKernelStartThread(rs_thread.id, 0, NULL);

    AppLog("[REWIND] buf size: 0x%08x, state_size: 0x%08x\n", buffer_size, rs.state_size);
    AppLog("[REWIND] blocks: 0x%08x buf: 0x%08x tmp_buf: 0x%08x\n", rs.blocks, rs.buf, rs.tmp_buf);
    AppLog("[REWIND] rewind init OK!\n");
}

void Emu_DeinitRewind()
{
    AppLog("[REWIND] rewind deinit...\n");
    if (rs_thread.id)
    {
        sceKernelLockLwMutex(&rs_thread.mutex, 1, NULL);
        sceKernelUnlockLwMutex(&rs_thread.mutex, 1);
    }

    AppLog("[REWIND] blocks: 0x%08x buf: 0x%08x tmp_buf: 0x%08x\n", rs.blocks, rs.buf, rs.tmp_buf);
    if (rs.blocks)
        free(rs.blocks);
    if (rs.buf)
        free(rs.buf);
    if (rs.tmp_buf)
        free(rs.tmp_buf);

    AppLog("[REWIND] free all\n");
    memset(&rs, 0, sizeof(RewindState));

    if (rs_thread.id)
    {
        sceKernelSignalSema(rs_thread.semaphore, 1);
        sceKernelWaitThreadEnd(rs_thread.id, NULL, NULL);
        sceKernelDeleteSema(rs_thread.semaphore);
        sceKernelDeleteLwMutex(&rs_thread.mutex);
        memset(&rs_thread, 0, sizeof(RewindSavingThread));
    }

    AppLog("[REWIND] rewind deinit OK!\n");
}

void Emu_RewindCheck()
{
    if (in_rewinding)
    {
        if (!Emu_IsVideoPaused())
            Emu_PauseVideo();
    }

    if ((!Emu_IsGameLoaded()) || rs.buf == NULL || sceKernelGetProcessTimeWide() < rs.next_time)
        return;

    if (rewind_key_pressed)
        Rewind();
    else
    {
        SaveState();
        if (in_rewinding)
        {
            Emu_ResumeCheat();
            Emu_ResumeAudio();
            if (Emu_IsVideoPaused())
                Emu_ResumeVideo();
            in_rewinding = 0;
        }
    }
    rs.next_time = sceKernelGetProcessTimeWide() + NEXT_STATE_PERIOD;
}