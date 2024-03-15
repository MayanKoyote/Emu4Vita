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
#include "config.h"
#include "activity/emu_activity.h"

#define ALIGN_UP(x, a) ((x) + ((a)-1)) & ~((a)-1)
#define ALIGN_UP_10(x) ALIGN_UP(x, 0x10)

// 每 0.05 秒记录一次
#define NEXT_STATE_PERIOD 50000
// 如果差异的大小超过 THRESHOLD_RATE，则保存全部
#define THRESHOLD_RATE 0.1
// 最小差异单位
#define DIFF_STEP 0x10
#define MEMCMP2(SIZE) memcmp_##SIZE
#define MEMCMP(SIZE) MEMCMP2(SIZE)
#define MEMCMP_DIFF_STEP MEMCMP(DIFF_STEP)
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

typedef struct
{
    uint32_t type;   // BLOCK_FULL 或 BLOCK_DIFF
    uint32_t index;  // 流水号, 为当前的 RewindState.count，赋值后 RewindState.count++
    uint8_t *offset; // 指向为于 RewindState.buf 中的 *RewindBufHeader 或 *RewindDiffBuf
    uint32_t size;   // 在 RewindState.buf 中的占用的大小
} RewindBlock;

// 用于判断保存的 state 是否还有效 （未被覆盖）
// magic 固定为 REWIND_BLOCK_MAGIC
// index 用于和 RewindBlock.index 进行比较
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
    uint8_t buf[]; // 完整的state
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
    // 后续接 uint8_t *diff_buf;
} RewindDiffBuf;

// 缓存的全局状态
typedef struct
{
    RewindBlock *blocks;          // 指向 RewindBlock[BLOCK_SIZE]
    RewindBlock *current_block;   // 指向当前的状态
    RewindBlock *last_full_block; // 最新的完整状态
    uint8_t *buf;                 // 分配得到的内存地址
    uint8_t *tmp_buf;             // 用于比较、恢复差异状态
    size_t buf_size;              // buf 缓存的大小
    size_t threshold_size;        // state_size * THRESHOLD_RATE, 如果差异的数据大于 threshold_size, 则保存完全的状态
    size_t state_size;            // 完整状态的实际大小
    size_t aligned_state_size;    // 完整状态大小，向上 0x10 对齐
    uint64_t next_time;           // 下一次记录的时间
    uint32_t count;               // 用于设置 RewindBlock 中的 index
} RewindState;

static RewindState rs = {0};

static int rewind_key_pressed = 0;
static SceUID rewind_thread_id = -1;
static int rewind_run = 0;
static SceUID rewind_semaphore = -1;

static RewindBlock *
GetNextBlock()
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
    if (buf + rs.aligned_state_size + sizeof(RewindFullBuf) >= rs.buf + rs.buf_size)
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

static int SerializeState(void *data, size_t size)
{
    Emu_LockRunGame();
    int result = retro_serialize(data, size);
    Emu_UnlockRunGame();
    return result;
}

static int UnserializeState(void *data, size_t size)
{
    Emu_LockRunGame();
    int result = retro_unserialize(data, size);
    Emu_UnlockRunGame();
    return result;
}

static int
SaveFullState(RewindBlock *block, uint8_t *buf_offset, uint8_t *state)
{
    block->type = BLOCK_FULL;
    block->index = rs.count++;
    block->offset = buf_offset;
    block->size = sizeof(RewindFullBuf) + rs.aligned_state_size;
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
        return SerializeState(full->buf, rs.state_size);
    // AppLog("SaveFullState %08x %08x\n", block, buf_offset);
}

// 不要用 uint64_t，会触发memcpy，造成负优化 !!
static inline int memcmp_0x10(const void *src, const void *dst)
{
    const uint32_t *s = src;
    const uint32_t *d = dst;
    return s[0] != d[0] || s[1] != d[1] || s[2] != d[2] || s[3] != d[3];
}

// 仅把需要复制的差异信息(地址, 大小)写入 areas，统计 total_size
// 调用前需先把 state 保存入 rs.tmp_buf
static void PreSaveDiffState(RewindBlock *block, uint8_t *buf_offset, RewindBlock *full_block)
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

    uint8_t *old = ((RewindFullBuf *)full_block->offset)->buf;
    uint8_t *new = rs.tmp_buf;
    int last_state = 0; // 相同: 0, 不同: 1
    int offset = 0;
    uint32_t tail_size = rs.state_size % DIFF_STEP;
    size_t loop_state_size = rs.state_size - tail_size;
    for (; offset < loop_state_size; offset += DIFF_STEP)
    {
        if (MEMCMP_DIFF_STEP(old + offset, new + offset) == 0)
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

    offset = loop_state_size;
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
    block->size = ALIGN_UP_10(block->size);
}

static uint8_t *GetDiffBuffer(const RewindDiffBuf *diff)
{
    return (uint8_t *)diff + sizeof(RewindDiffBuf) + diff->num * sizeof(DiffArea);
}

static void SaveDiffState(RewindBlock *block)
{
    RewindDiffBuf *diff = (RewindDiffBuf *)block->offset;
    uint8_t *buf = GetDiffBuffer(diff);
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

    uint8_t *buf = GetDiffBuffer(diff);
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

static size_t GetBufDistance(size_t first, size_t second)
{
    if (second >= first)
        return second - first;
    else
        return rs.buf_size + first - second;
}

static void Rewind()
{
    // AppLog("Rewind\n");
    RewindBlock *prev = GetPreviousBlock();
    // AppLog("%08x %08x\n", rs.current_block, prev);
    if (IsValidBlock(prev))
    {
        void *state = GetState(rs.current_block);
        if (state)
        {
            UnserializeState(state, rs.state_size);
            rs.current_block = prev;
        }
    }

    sceKernelSignalSema(rewind_semaphore, 1);
    // AppLog("Rewind End\n");
}

static void SaveState()
{
    // AppLog("SaveState\n");
    if (rs.current_block == NULL)
    {
        // 进游戏后首次保存
        if (SaveFullState(rs.blocks, rs.buf, NULL))
            rs.current_block = rs.blocks;
        return;
    }

    RewindBlock *current = rs.current_block;
    RewindBlock *next = GetNextBlock();
    uint8_t *next_buf = GetNextBuf();
    int result = 0;

    if (GetBufDistance((size_t)rs.last_full_block->offset, (size_t)next_buf) > rs.buf_size / 2)
        // 确保至少有两个完整的 state
        result = SaveFullState(next, next_buf, NULL);
    else
    {
        result = SerializeState(rs.tmp_buf, rs.state_size);
        if (result)
        {
            RewindBlock *full_block = current->type == BLOCK_FULL ? current : ((RewindDiffBuf *)(current->offset))->full_block;
            PreSaveDiffState(next, next_buf, full_block);
            if (next->size > rs.threshold_size)
                result = SaveFullState(next, next_buf, rs.tmp_buf);
            else
                SaveDiffState(next);
        }
    }

    if (result)
        rs.current_block = next;
}

static int RewindThreadFunc()
{
    while (rewind_run)
    {
        if (!Emu_IsGameRunning())
        {
            sceKernelDelayThread(100000);
            continue;
        }

        int current_time = sceKernelGetProcessTimeWide();
        if (current_time < rs.next_time)
            sceKernelDelayThread(rs.next_time - current_time);

        rewind_key_pressed ? Rewind() : SaveState();

        uint64_t period = MAX(NEXT_STATE_PERIOD, Emu_GetMicrosPerFrame());
        rs.next_time = current_time + period;
    }

    AppLog("RewindThreadFunc end\n");
    return 0;
}

void Emu_InitRewind()
{
    if (!misc_config.enable_rewind)
        return;

    AppLog("[REWIND] rewind init...\n");

    memset(&rs, 0, sizeof(RewindState));

    size_t buffer_size = misc_config.rewind_buffer_size << 20;
    rs.state_size = retro_serialize_size();
    rs.aligned_state_size = ALIGN_UP_10(rs.state_size);
    if (buffer_size < rs.aligned_state_size * MIN_STATE_RATE)
    {
        AppLog("[REWIND] the buffer size is too small, Minimum required is %DMB", (rs.aligned_state_size * MIN_STATE_RATE) >> 20);
        AppLog("[REWIND] rewind init failed\n");
        return;
    }

    rs.buf_size = buffer_size - sizeof(RewindBlock) * BLOCK_SIZE - rs.aligned_state_size;
    rs.threshold_size = rs.aligned_state_size * THRESHOLD_RATE;

    rs.blocks = calloc(BLOCK_SIZE, sizeof(RewindBlock));
    rs.buf = calloc(1, rs.buf_size);
    rs.tmp_buf = calloc(1, rs.aligned_state_size);

    rewind_run = 1;
    rewind_thread_id = sceKernelCreateThread("rewind_main_thread", RewindThreadFunc, 191, 0x10000, 0, 0, NULL);
    rewind_semaphore = sceKernelCreateSema("rewind_loading_semaphore", 0, 0, 1, NULL);

    if (!(rs.blocks && rs.buf && rs.tmp_buf && rewind_thread_id >= 0 && rewind_semaphore >= 0))
    {
        AppLog("[REWIND] rewind init failed\n");
        Emu_DeinitRewind();
        return;
    }

    sceKernelStartThread(rewind_thread_id, 0, NULL);

    // AppLog("[REWIND] buf size: 0x%08x, state_size: 0x%08x aligned_state_size: 0x%08x\n", buffer_size, rs.state_size, rs.aligned_state_size);
    // AppLog("[REWIND] blocks: 0x%08x buf: 0x%08x tmp_buf: 0x%08x\n", rs.blocks, rs.buf, rs.tmp_buf);
    AppLog("[REWIND] rewind init OK!\n");
}

void Emu_DeinitRewind()
{
    AppLog("[REWIND] rewind deinit...\n");
    rewind_run = 0;
    if (rewind_thread_id >= 0)
    {
        sceKernelWaitThreadEnd(rewind_thread_id, NULL, NULL);
        sceKernelDeleteThread(rewind_thread_id);
        rewind_thread_id = -1;
    }

    if (rewind_semaphore >= 0)
    {
        sceKernelDeleteSema(rewind_semaphore);
        rewind_semaphore = -1;
    }

    // AppLog("[REWIND] blocks: 0x%08x buf: 0x%08x tmp_buf: 0x%08x\n", rs.blocks, rs.buf, rs.tmp_buf);
    if (rs.blocks)
        free(rs.blocks);
    if (rs.buf)
        free(rs.buf);
    if (rs.tmp_buf)
        free(rs.tmp_buf);

    memset(&rs, 0, sizeof(RewindState));

    AppLog("[REWIND] rewind deinit OK!\n");
}

void Emu_StartRewindGame()
{
    // AppLog("Emu_StartRewindGame\n");
    Emu_SetGameRunEventAction(TYPE_GAME_RUN_EVENT_ACTION_START_REWIND);
    rewind_key_pressed = 1;
}

void Emu_StopRewindGame()
{
    // AppLog("Emu_StopRewindGame\n");
    Emu_SetGameRunEventAction(TYPE_GAME_RUN_EVENT_ACTION_STOP_REWIND);
    rewind_key_pressed = 0;
}

int Emu_IsInRewinding()
{
    return rewind_key_pressed;
}

void Emu_WaitRewind()
{
    int result = sceKernelWaitSema(rewind_semaphore, 1, NULL);
    if (result != SCE_OK)
    {
        AppLog("[REWIND] sceKernelWaitSema Error: %d\n", result);
    }
}