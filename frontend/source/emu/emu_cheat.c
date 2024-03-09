#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <psp2/kernel/threadmgr/thread.h>

#include "activity/activity.h"
#include "list/cheat_list.h"
#include "setting/setting.h"
#include "emu/emu.h"
#include "config.h"
#include "utils.h"

static SceUID cheat_thid = -1;
static int cheat_run = 0;
static int cheat_pause = 1;
static int cheat_reset = 0;

static int makeCheatPath(char *path)
{
    char name[MAX_NAME_LENGTH];
    MakeCurrentFileName(name);
    char base_name[MAX_NAME_LENGTH];
    MakeBaseName(base_name, name, MAX_NAME_LENGTH);
    snprintf(path, MAX_PATH_LENGTH, "%s/%s.cht", (CORE_CHEATS_DIR), base_name);
    return 0;
}

static int makeCheatPath2(char *path)
{
    MakeCurrentFilePath(path);
    char *p = strrchr(path, '.');
    if (!p++)
        p = path + strlen(path);
    strcpy(p, "cht");
    return 0;
}

void Emu_PauseCheat()
{
    cheat_pause = 1;
}

void Emu_ResumeCheat()
{
    cheat_pause = 0;
}

void Emu_CleanCheatOption()
{
    Setting_SetCheatMenu(NULL);
    if (core_cheat_list)
        LinkedListDestroy(core_cheat_list);
    core_cheat_list = NULL;
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
    Emu_CleanCheatOption();

    core_cheat_list = NewCheatList();
    if (!core_cheat_list)
        goto FAILED;

    char path[1024];
    makeCheatPath2(path);
    if (CheatListGetEntries(core_cheat_list, path) < 0)
    {
        AppLog("[CHEAT] CheatListGetEntries failed: %s\n", path);
        makeCheatPath(path);
        CheatListGetEntries(core_cheat_list, path);
    }
    if (LinkedListGetLength(core_cheat_list) <= 0)
    {
        AppLog("[CHEAT] CheatListGetEntries failed: %s\n", path);
        goto FAILED;
    }
    AppLog("[CHEAT] CheatListGetEntries OK: %s\n", path);

    MakeConfigPath(path, CHEAT_CONFIG_NAME, TYPE_CONFIG_GAME);
    CheatListLoadConfig(core_cheat_list, path);

    Setting_SetCheatMenu(core_cheat_list);
    AppLog("[CHEAT] Emu_LoadCheatOption OK!\n");
    return 0;

FAILED:
    AppLog("[CHEAT] Emu_LoadCheatOption failed!\n");
    return -1;
}

int Emu_SaveCheatOption()
{
    if (!core_cheat_list)
        goto FAILED;

    char path[1024];
    MakeConfigPath(path, CHEAT_CONFIG_NAME, TYPE_CONFIG_GAME);
    if (CheatListSaveConfig(core_cheat_list, path) < 0)
        goto FAILED;

    AppLog("[CHEAT] Emu_SaveCheatOption OK!\n");
    return 0;

FAILED:
    AppLog("[CHEAT] Emu_SaveCheatOption failed!\n");
    return -1;
}

static int ApplyCheatOption()
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
                // printf("[CHEAT] ApplyCheatOption: %s = %s\n", data->desc, data->code);
                retro_cheat_set(index, 1, data->code);
            }
        }

        entry = LinkedListNext(entry);
        index++;
    }

    return 0;
}

static int ApplyCheatOptionThreadFunc(SceSize args, void *argp)
{
    AppLog("[CHEAT] Cheat thread start.\n");

    while (cheat_run)
    {
        if (cheat_pause || !core_cheat_list || LinkedListGetLength(core_cheat_list) <= 0)
        {
            sceKernelDelayThread(1000);
            continue;
        }

        ApplyCheatOption();
        sceKernelDelayThread(1000);
    }

    AppLog("[CHEAT] Cheat thread exit.\n");
    sceKernelExitThread(0);
    return 0;
}

static int ExitApplyCheatOptiontThread()
{
    if (cheat_thid >= 0)
    {
        cheat_run = 0;
        sceKernelWaitThreadEnd(cheat_thid, NULL, NULL);
        sceKernelDeleteThread(cheat_thid);
        cheat_thid = -1;
    }

    return 0;
}

static int StartApplyCheatOptionThread()
{
    int ret = -1;

    if (cheat_thid >= 0)
        ExitApplyCheatOptiontThread();

    ret = cheat_thid = sceKernelCreateThread("cheat_thread", ApplyCheatOptionThreadFunc, 0x10000100, 0x10000, 0, 0, NULL);
    if (cheat_thid >= 0)
    {
        cheat_run = 1;
        ret = sceKernelStartThread(cheat_thid, 0, NULL);
    }

    return ret;
}

int Emu_InitCheat()
{
    retro_cheat_reset();
    cheat_reset = 0;
    cheat_pause = 1;

    if (Emu_LoadCheatOption() < 0)
        return -1;

    return StartApplyCheatOptionThread();
}

int Emu_DeinitCheat()
{
    ExitApplyCheatOptiontThread();
    Emu_CleanCheatOption();

    return 0;
}
