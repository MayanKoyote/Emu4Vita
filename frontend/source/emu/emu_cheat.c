#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <psp2/kernel/processmgr.h>

#include "activity/activity.h"
#include "list/cheat_list.h"
#include "setting/setting.h"
#include "emu/emu.h"
#include "config.h"
#include "utils.h"

static SceKernelLwMutexWork cheat_mutex = {0};
static SceUID cheat_thid = -1;
static int cheat_okay = 0;
static int cheat_run = 0;
static int cheat_pause = 1;
static int cheat_reset = 0;

void Emu_PauseCheat()
{
    if (Emu_IsGameExiting())
    {
        sceKernelLockLwMutex(&cheat_mutex, 1, NULL);
        cheat_pause = 1;
        sceKernelUnlockLwMutex(&cheat_mutex, 1);
    }
    else
    {
        cheat_pause = 1;
    }
}

void Emu_ResumeCheat()
{
    cheat_pause = 0;
}

int Emu_CleanCheatOption()
{
    Setting_SetCheatMenu(NULL);
    LinkedListDestroy(core_cheat_list);
    core_cheat_list = NULL;

    return 0;
}

int Emu_UpdateCheatOption()
{
    cheat_reset = 1;

    return 0;
}

int Emu_ResetCheatOption()
{
    return CheatListResetConfig(core_cheat_list);
}

int Emu_LoadCheatOption()
{
    char path[1024];

    if (!core_cheat_list)
    {
        core_cheat_list = NewCheatList();
        if (!core_cheat_list)
            return -1;
    }

    MakeCheatPath2(path);
    if (CheatListGetEntries(core_cheat_list, path) < 0)
    {
        MakeCheatPath(path);
        CheatListGetEntries(core_cheat_list, path);
    }
    if (LinkedListGetLength(core_cheat_list) <= 0)
        return -1;

    MakeConfigPath(path, CHEAT_CONFIG_NAME, TYPE_CONFIG_GAME);
    CheatListLoadConfig(core_cheat_list, path);
    Setting_SetCheatMenu(core_cheat_list);

    return 0;
}

int Emu_SaveCheatOption()
{
    if (!core_cheat_list)
        return -1;

    char path[1024];
    MakeConfigPath(path, CHEAT_CONFIG_NAME, TYPE_CONFIG_GAME);
    return CheatListSaveConfig(core_cheat_list, path);
}

int Emu_ApplyCheatOption()
{
    if (cheat_reset)
    {
        retro_cheat_reset();
        cheat_reset = 0;
    }

    if (!core_cheat_list || LinkedListGetLength(core_cheat_list) <= 0)
        return -1;

    LinkedListEntry *entry = LinkedListHead(core_cheat_list);
    int index = 0;

    while (entry)
    {
        CheatListEntryData *data = (CheatListEntryData *)LinkedListGetEntryData(entry);
        if (data->enable)
        {
            if (data->code)
            {
                // printf("[CHEAT] Emu_ApplyCheatOption: %s = %s\n", data->desc, data->code);
                retro_cheat_set(index, 1, data->code);
            }
        }

        entry = LinkedListNext(entry);
        index++;
    }

    return 0;
}

static int CheatThreadEntry(SceSize args, void *argp)
{
    APP_LOG("[CHEAT] Cheat thread start run.\n");

    while (cheat_run)
    {
        sceKernelLockLwMutex(&cheat_mutex, 1, NULL);

        if (cheat_pause || LinkedListGetLength(core_cheat_list) <= 0)
        {
            sceKernelUnlockLwMutex(&cheat_mutex, 1);
            sceKernelDelayThread(1000);
            continue;
        }

        uint64_t next_micros = sceKernelGetProcessTimeWide() + Emu_GetMicrosPerFrame();
        Emu_LockRunGameMutex();
        Emu_ApplyCheatOption();
        Emu_UnlockRunGameMutex();

        sceKernelUnlockLwMutex(&cheat_mutex, 1);

        uint64_t cur_micros = sceKernelGetProcessTimeWide();
        if (cur_micros < next_micros)
            sceKernelDelayThread(next_micros - cur_micros);
    }

    APP_LOG("[CHEAT] Cheat thread stop run.\n");
    sceKernelExitThread(0);
    return 0;
}

static int startCheatThread()
{
    int ret = 0;

    if (cheat_thid < 0)
        ret = cheat_thid = sceKernelCreateThread("emu_cheat_thread", CheatThreadEntry, 0x10000100, 0x10000, 0, 0, NULL);
    if (cheat_thid >= 0)
    {
        cheat_run = 1;
        ret = sceKernelStartThread(cheat_thid, 0, NULL);
        if (ret < 0)
        {
            cheat_run = 0;
            sceKernelDeleteThread(cheat_thid);
            cheat_thid = -1;
        }
    }

    return ret;
}

static int finishCheatThread()
{
    cheat_run = 0;
    if (cheat_thid >= 0)
    {
        sceKernelWaitThreadEnd(cheat_thid, NULL, NULL);
        sceKernelDeleteThread(cheat_thid);
        cheat_thid = -1;
    }

    return 0;
}

int Emu_InitCheat()
{
    if (cheat_okay)
        Emu_DeinitCheat();

    APP_LOG("[CHEAT] Cheat init...\n");

    if (Emu_LoadCheatOption() < 0)
        goto FAILED;

    sceKernelCreateLwMutex(&cheat_mutex, "emu_cheat_mutex", 2, 0, NULL);

    cheat_pause = 1;
    if (startCheatThread() < 0)
        goto FAILED_DEINIT;

    cheat_okay = 1;
    APP_LOG("[CHEAT] Cheat init OK!\n");
    return 0;

FAILED_DEINIT:
    Emu_DeinitCheat();
FAILED:
    APP_LOG("[CHEAT] Cheat init failed!\n");
    return -1;
}

int Emu_DeinitCheat()
{
    APP_LOG("[CHEAT] Cheat deinit...\n");

    cheat_okay = 0;
    finishCheatThread();
    sceKernelDeleteLwMutex(&cheat_mutex);
    Emu_CleanCheatOption();

    APP_LOG("[CHEAT] Cheat deinit OK!\n");
    return 0;
}
