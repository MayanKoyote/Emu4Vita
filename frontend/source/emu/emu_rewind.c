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

typedef struct RewindEntryData
{
    void *state_data;
    size_t state_size;
} RewindEntryData;

#define MICROS_PER_SECOND 1000000llu

static SceUID rewind_thid = -1;
static LinkedList *rewind_list = NULL;
static uint64_t last_save_micros = 0;
static int rewind_run = 0, rewind_pause = 0;
static int rewind_serializing = 0;
static int last_rewind_event = TYPE_REWIND_EVENT_NONE;

static void freeRewindEntryData(void *data)
{
    if (data)
    {
        RewindEntryData *r_data = (RewindEntryData *)data;
        if (r_data->state_data)
            free(r_data->state_data);
        free(data);
    }
}

LinkedList *NewRewindList()
{
    LinkedList *list = NewLinkedList();
    if (!list)
        return NULL;

    LinkedListSetFreeEntryDataCallback(list, freeRewindEntryData);

    return list;
}

static int RewindSaveState()
{
    int ret = 0;

    Emu_LockRunGame();      // 游戏运行锁，无法在游戏运行中保存存档，需要加互斥锁
    rewind_serializing = 1; // 有些核心保存即时存档时会调用RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE获取是否要保存音视频等，rewind可以不要音视频节省容量和时间

    size_t state_size = retro_serialize_size();
    if (state_size == 0)
        goto FAILED;

    void *state_data = malloc(state_size);
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
        free(state_data);
        goto FAILED;
    }

    RewindEntryData *data = (RewindEntryData *)calloc(1, sizeof(RewindEntryData));
    if (!data)
    {
        free(state_data);
        goto FAILED;
    }
    data->state_data = state_data;
    data->state_size = state_size;

    // 存档已满，删除最早的存档
    while (LinkedListGetLength(rewind_list) >= misc_config.rewind_max_count)
    {
        LinkedListEntry *head = LinkedListHead(rewind_list);
        LinkedListRemove(rewind_list, head);
    }

    last_rewind_event = TYPE_REWIND_EVENT_SAVE;
    last_save_micros = sceKernelGetProcessTimeWide();
    // printf("[REWIND] RewindSaveState: index=%d, size=%u\n", LinkedListGetLength(rewind_list), data->state_size);
    LinkedListAdd(rewind_list, data);

END:
    rewind_serializing = 0;
    Emu_UnlockRunGame();
    return ret;

FAILED:
    ret = -1;
    goto END;
}

static int RewindLoadState()
{
    int ret = 0;

    Emu_LockRunGame();
    rewind_serializing = 1;

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

    RewindEntryData *data = LinkedListGetEntryData(entry);
    if (!data)
        goto FAILED;

    if (!retro_unserialize(data->state_data, data->state_size))
        goto FAILED;

    last_rewind_event = TYPE_REWIND_EVENT_LOAD;
    last_save_micros = sceKernelGetProcessTimeWide();
    // printf("[REWIND] RewindLoadState: index=%d, size=%u\n", LinkedListGetLength(rewind_list), data->state_size);

END:
    rewind_serializing = 0;
    Emu_UnlockRunGame();
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
        if (rewind_pause || !rewind_list || !misc_config.enable_rewind)
        {
            sceKernelDelayThread(1000);
            last_save_micros += 1000; // last_save_micros跟随休眠时间增加以对应正确的rewind_interval_micros
            continue;
        }

        uint64_t cur_micros = sceKernelGetProcessTimeWide();
        uint64_t rewind_interval_micros = misc_config.rewind_interval_time * MICROS_PER_SECOND;
        uint64_t cur_interval_micros = cur_micros - last_save_micros;
        if (cur_interval_micros < rewind_interval_micros)
        {
            // 不休眠全部时间差，最大只休眠1000，continue重新判断。因为休眠时有可能会打开菜单，如此会造成真实的游戏运行间隔时间出现错误
            uint64_t delay_micros = MIN(rewind_interval_micros - cur_interval_micros, 1000);
            sceKernelDelayThread(delay_micros);
            continue;
        }

        RewindSaveState();
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
    rewind_run = 0;
    if (rewind_thid >= 0)
    {
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
    return RewindLoadState();
}

int Emu_InitRewind()
{
    if (!rewind_list)
    {
        rewind_list = NewRewindList();
        if (!rewind_list)
            return -1;
    }
    else
    {
        LinkedListEmpty(rewind_list);
    }

    last_rewind_event = TYPE_REWIND_EVENT_NONE;
    rewind_pause = 1;
    rewind_run = 0;
    if (startRewindThread() < 0)
        return -1;

    return 0;
}

int Emu_DeinitRewind()
{
    finishRewindThread();
    LinkedListDestroy(rewind_list);
    rewind_list = NULL;

    return 0;
}
