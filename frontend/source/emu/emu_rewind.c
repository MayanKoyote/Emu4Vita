#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <psp2/kernel/threadmgr/thread.h>
#include <psp2/kernel/processmgr.h>

#include "gui/gui.h"
#include "emu.h"
#include "utils.h"
#include "config.h"

typedef enum RewindEventType
{
    TYPE_REWIND_EVENT_NONE,
    TYPE_REWIND_EVENT_SAVE,
    TYPE_REWIND_EVENT_LOAD,
} RewindEventType;

typedef struct BlockTableEntryData
{
    size_t offset;
    size_t size;
} BlockTableEntryData;

typedef struct RewindState
{
    void *block_data;
    size_t block_size;
    LinkedList *table_list;
    size_t state_size;
    void *parent_data;
    size_t parent_size;
    LinkedListEntry *entry;
} RewindState;

#define MICROS_PER_SECOND 1000000llu

static SceKernelLwMutexWork rewind_mutex = {0};
static SceUID rewind_thid = -1;
static LinkedList *rewind_list = NULL;
static uint64_t last_save_micros = 0;
static int rewind_okay = 0;
static int rewind_run = 0, rewind_pause = 0;
static int last_rewind_event = TYPE_REWIND_EVENT_NONE;

static void freeRewindEntryData(void *data)
{
    if (data)
    {
        RewindState *state = (RewindState *)data;

        if (state->block_data)
            free(state->block_data);

        if (state->table_list)
            LinkedListDestroy(state->table_list);

        if (state->parent_data) // 如果无其它条目在使用parent_data，释放parent_data
        {
            LinkedListEntry *prev_entry = LinkedListPrev(state->entry);
            RewindState *prev_state = (RewindState *)LinkedListGetEntryData(prev_entry);
            LinkedListEntry *next_entry = LinkedListNext(state->entry);
            RewindState *next_state = (RewindState *)LinkedListGetEntryData(next_entry);
            if ((!prev_state || prev_state->parent_data != state->parent_data) && (!next_state || next_state->parent_data != state->parent_data))
            {
                // printf("[REWIND] freeRewindEntryData: free parent_data: %p\n", state->parent_data);
                free(state->parent_data);
            }
        }

        free(state);
    }
}

static LinkedList *NewRewindList()
{
    LinkedList *list = NewLinkedList();
    if (!list)
        return NULL;

    LinkedListSetFreeEntryDataCallback(list, freeRewindEntryData);

    return list;
}

static RewindState *RewindCreateState(void *current_data, size_t current_size, void *parent_data, size_t parent_size)
{
    if (!current_data)
        return NULL;

    RewindState *state = (RewindState *)calloc(1, sizeof(RewindState));
    if (!state)
        return NULL;

    if (!parent_data)
        goto SAVE_FULL;

    state->state_size = current_size;
    state->parent_data = parent_data;
    state->parent_size = parent_size;

    state->table_list = NewLinkedList();
    if (!state->table_list)
        goto SAVE_FULL;
    LinkedListSetFreeEntryDataCallback(state->table_list, free);

    uint64_t *current_data64 = (uint64_t *)current_data;
    uint64_t *parent_data64 = (uint64_t *)parent_data;
    size_t compare_size = MIN(current_size, parent_size);
    size_t compare_step = sizeof(uint64_t);
    size_t remaining_size = compare_size % compare_step;
    size_t main_size = compare_size - remaining_size;
    size_t remaining_offset = main_size;
    size_t max_total_diff_size = parent_size / 10;
    size_t total_diff_size = 0;
    size_t diff_size = 0;
    size_t diff_offset = 0;

    int i;
    for (i = 0; i < main_size / compare_step; i++)
    {
        if (current_data64[i] != parent_data64[i])
        {
            if (diff_size == 0)
                diff_offset = i * compare_step;
            diff_size += compare_step;
            total_diff_size += compare_step;
            if (total_diff_size > max_total_diff_size)
                goto SAVE_FULL;
        }
        else
        {
            if (diff_size > 0)
            {
                BlockTableEntryData *data = (BlockTableEntryData *)calloc(1, sizeof(BlockTableEntryData));
                if (!data)
                    goto SAVE_FULL;
                data->size = diff_size;
                data->offset = diff_offset;
                LinkedListAdd(state->table_list, data);
                diff_size = 0;
            }
        }
    }

    if (current_size > parent_size)
        remaining_size += (current_size - parent_size);

    total_diff_size += remaining_size;
    if (total_diff_size > max_total_diff_size)
        goto SAVE_FULL;

    if (diff_size > 0) // main_size里剩余的未保存的
    {
        remaining_size += diff_size;
        remaining_offset = diff_offset;
    }

    if (remaining_size > 0)
    {
        BlockTableEntryData *data = (BlockTableEntryData *)calloc(1, sizeof(BlockTableEntryData));
        if (!data)
            goto SAVE_FULL;
        data->size = remaining_size;
        data->offset = remaining_offset;
        LinkedListAdd(state->table_list, data);
    }

    state->block_size = total_diff_size;
    state->block_data = malloc(state->block_size);
    if (!state->block_data)
        goto SAVE_FULL;

    // 保存有差异的数据块
    size_t block_offset = 0;
    LinkedListEntry *entry = LinkedListHead(state->table_list);
    while (entry)
    {
        BlockTableEntryData *data = (BlockTableEntryData *)LinkedListGetEntryData(entry);
        memcpy(state->block_data + block_offset, current_data + data->offset, data->size);
        block_offset += data->size;
        entry = LinkedListNext(entry);
    }

    return state;

SAVE_FULL:
    if (state)
    {
        if (state->block_data)
        {
            free(state->block_data);
            state->block_data = NULL;
        }
        if (state->table_list)
        {
            LinkedListDestroy(state->table_list);
            state->table_list = NULL;
        }
        state->state_size = current_size;
        state->parent_data = current_data;
        state->parent_size = current_size;
    }
    return state;
}

static int RewindGetStateData(RewindState *state, void **data, size_t *size)
{
    if (!state || !state->parent_data || !data || !size)
        return -1;

    if (!state->block_data || !state->table_list) // 是个完整的存档
    {
        *data = state->parent_data;
        *size = state->parent_size;
        return 0;
    }

    size_t state_size = state->state_size;
    void *state_data = malloc(state_size);
    if (!state_data)
        return -1;

    // 先复制parent_data
    memcpy(state_data, state->parent_data, MIN(state_size, state->parent_size));

    // 覆盖有差异的数据块
    size_t block_offset = 0;
    LinkedListEntry *entry = LinkedListHead(state->table_list);
    while (entry)
    {
        BlockTableEntryData *data = (BlockTableEntryData *)LinkedListGetEntryData(entry);
        memcpy(state_data + data->offset, state->block_data + block_offset, data->size);
        block_offset += data->size;
        entry = LinkedListNext(entry);
    }

    *data = state_data;
    *size = state_size;

    return 0;
}

static int RewindSaveState()
{
    int ret = 0;
    RewindState *state = NULL;
    void *state_data = NULL;
    size_t state_size = 0;

    if (!rewind_okay)
        goto FAILED;

    Emu_LockRunGame();

    state_size = retro_serialize_size();
    if (state_size == 0)
    {
        Emu_UnlockRunGame();
        goto FAILED;
    }

    state_data = malloc(state_size);
    if (!state_data)
    {
        GUI_ShowToast("[REWIND] Failed to malloc state data, please try to reduce the max rewind count!", 2); // 无法分配内存
        while (!state_data && LinkedListGetLength(rewind_list) > 0)
        {
            LinkedListEntry *head = LinkedListHead(rewind_list);
            LinkedListRemove(rewind_list, head); // 释放旧存档
            state_data = malloc(state_size);
        }
        if (!state_data)
            goto FAILED;
    }
    if (!retro_serialize(state_data, state_size))
    {
        Emu_UnlockRunGame();
        goto FAILED;
    }

    Emu_UnlockRunGame();

    void *parent_data = NULL;
    size_t parent_size = 0;

    LinkedListEntry *tail = LinkedListTail(rewind_list);
    RewindState *tail_state = (RewindState *)LinkedListGetEntryData(tail);
    if (tail_state)
    {
        parent_data = tail_state->parent_data;
        parent_size = tail_state->parent_size;
    }

    state = RewindCreateState(state_data, state_size, parent_data, parent_size);
    if (!state)
        goto FAILED;

    // 存档已满，删除最早的存档
    while (LinkedListGetLength(rewind_list) >= misc_config.rewind_max_count)
    {
        LinkedListEntry *head = LinkedListHead(rewind_list);
        LinkedListRemove(rewind_list, head);
    }
    state->entry = LinkedListAdd(rewind_list, state);

    last_rewind_event = TYPE_REWIND_EVENT_SAVE;
    last_save_micros = sceKernelGetProcessTimeWide();
    // printf("[REWIND] RewindSaveState: index: %d, block_size: %u, table_length: %d, parent_size: %u\n", LinkedListGetLength(rewind_list), state->block_size, LinkedListGetLength(state->table_list), state->parent_size);

END:
    if (state_data && (!state || state->parent_data != state_data))
        free(state_data);
    return ret;

FAILED:
    ret = -1;
    goto END;
}

static int RewindLoadState()
{
    int ret = 0;
    RewindState *state = NULL;
    void *state_data = NULL;
    size_t state_size = 0;

    if (!rewind_okay)
        goto FAILED;

    LinkedListEntry *entry = LinkedListTail(rewind_list);
    if (!entry)
        goto END;

    if (last_rewind_event == TYPE_REWIND_EVENT_LOAD)
    {
        // 如果是连续回退读取存档，且在当前存档前面还有存档，则删除当前存档，读取前面的存档
        LinkedListEntry *prev = LinkedListPrev(entry);
        if (prev)
        {
            LinkedListRemove(rewind_list, entry);
            entry = prev;
        }
    }

    state = (RewindState *)LinkedListGetEntryData(entry);
    if (!state)
        goto FAILED;

    if (RewindGetStateData(state, &state_data, &state_size) < 0)
        goto FAILED;

    if (!retro_unserialize(state_data, state_size))
        goto FAILED;

    last_rewind_event = TYPE_REWIND_EVENT_LOAD;
    last_save_micros = sceKernelGetProcessTimeWide();
    // printf("[REWIND] RewindLoadState: index: %d, block_size: %u, table_length: %d, parent_size: %u\n", LinkedListGetLength(rewind_list), state->block_size, LinkedListGetLength(state->table_list), state->parent_size);

END:
    if (state_data && (!state || state->parent_data != state_data))
        free(state_data);
    return ret;

FAILED:
    ret = -1;
    goto END;
}

static int rewindThreadEntry(SceSize args, void *argp)
{
    AppLog("[REWIND] Rewind thread start.\n");
    last_save_micros = sceKernelGetProcessTimeWide();

    while (rewind_run)
    {
        if (rewind_pause || !rewind_okay || !misc_config.enable_rewind)
        {
            sceKernelDelayThread(1000);
            last_save_micros += 1000; // last_save_micros跟随休眠时间增加以对应正确的rewind_interval_micros
            continue;
        }

        sceKernelLockLwMutex(&rewind_mutex, 1, NULL);
        uint64_t cur_micros = sceKernelGetProcessTimeWide();
        uint64_t rewind_interval_micros = (uint64_t)misc_config.rewind_interval_time * MICROS_PER_SECOND;
        uint64_t cur_interval_micros = cur_micros - last_save_micros;
        if (cur_interval_micros < rewind_interval_micros)
        {
            sceKernelUnlockLwMutex(&rewind_mutex, 1);
            // 不休眠全部时间差，最大只休眠1000，continue重新判断。因为休眠时有可能会打开菜单，如此会造成真实的游戏运行间隔时间出现错误
            uint64_t delay_micros = MIN(rewind_interval_micros - cur_interval_micros, 1000);
            sceKernelDelayThread(delay_micros);
            continue;
        }

        RewindSaveState();
        sceKernelUnlockLwMutex(&rewind_mutex, 1);
    }

    AppLog("[REWIND] Rewind thread exit.\n");
    sceKernelExitThread(0);
    return 0;
}

static int startRewindThread()
{
    int ret = 0;

    if (rewind_thid < 0)
    {
        ret = rewind_thid = sceKernelCreateThread("emu_rewind_thread", rewindThreadEntry, 0x10000100, 0x10000, 0, 0, NULL);
        if (rewind_thid >= 0)
        {
            rewind_run = 1;
            ret = sceKernelStartThread(rewind_thid, 0, NULL);
            if (ret < 0)
            {
                sceKernelDeleteThread(rewind_thid);
                rewind_thid = -1;
                rewind_run = 0;
            }
        }
    }

    return ret;
}

static int finishRewindThread()
{
    if (rewind_thid >= 0)
    {
        rewind_run = 0;
        sceKernelWaitThreadEnd(rewind_thid, NULL, NULL);
        sceKernelDeleteThread(rewind_thid);
        rewind_thid = -1;
    }

    return 0;
}

void Emu_PauseRewind()
{
    rewind_pause = 1;
}

void Emu_ResumeRewind()
{
    rewind_pause = 0;
}

int Emu_RewindGame()
{
    int ret = 0;

    if (rewind_okay && misc_config.enable_rewind)
    {
        sceKernelLockLwMutex(&rewind_mutex, 1, NULL);
        ret = RewindLoadState();
        sceKernelUnlockLwMutex(&rewind_mutex, 1);
    }

    return ret;
}

int Emu_InitRewind()
{
    if (rewind_okay)
        Emu_DeinitRewind();

    if (!misc_config.enable_rewind)
        return 0;

    AppLog("[REWIND] Rewind init...\n");

    rewind_list = NewRewindList();
    if (!rewind_list)
        goto FAILED;

    // 保存与读取的互斥锁
    sceKernelCreateLwMutex(&rewind_mutex, "emu_rewind_mutex", 2, 0, NULL);

    last_rewind_event = TYPE_REWIND_EVENT_NONE;
    rewind_pause = 1;
    if (startRewindThread() < 0)
        goto FAILED;
    rewind_okay = 1;

    AppLog("[REWIND] Rewind init OK!\n");
    return 0;

FAILED:
    AppLog("[REWIND] Rewind init failed!\n");
    Emu_DeinitRewind();
    return -1;
}

int Emu_DeinitRewind()
{
    AppLog("[REWIND] Rewind deinit...\n");

    finishRewindThread();
    sceKernelDeleteLwMutex(&rewind_mutex);
    LinkedListDestroy(rewind_list);
    rewind_list = NULL;
    rewind_okay = 0;

    AppLog("[REWIND] Rewind deinit OK!\n");

    return 0;
}
