#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "activity/browser.h"
#include "list/cheat_list.h"
#include "setting/setting.h"
#include "emu/emu.h"
#include "config.h"
#include "utils.h"

static int makeCheatPath(char *path)
{
    char name[MAX_NAME_LENGTH];
    MakeCurrentFileName(name);
    char base_name[MAX_NAME_LENGTH];
    MakeBaseName(base_name, name, MAX_NAME_LENGTH);
    snprintf(path, MAX_PATH_LENGTH, "%s/%s.cht", (CORE_CHEATS_DIR), base_name);
    return 0;
}

int Emu_LoadCheatOption()
{
    AppLog("[CHEAT] Emu_LoadCheatOption...\n");

    Setting_SetCheatMenu(NULL);
    if (core_cheat_list)
        LinkedListDestroy(core_cheat_list);
    core_cheat_list = NULL;

    char path[1024];
    makeCheatPath(path);
    core_cheat_list = NewCheatList();
    if (!core_cheat_list)
        goto FAILED;

    CheatListGetEntries(core_cheat_list, path);
    if (LinkedListGetLength(core_cheat_list) <= 0)
        goto FAILED;

    Setting_SetCheatMenu(core_cheat_list);
    AppLog("[CHEAT] Emu_LoadCheatOption OK!\n");
    return 0;

FAILED:
    AppLog("[CHEAT] Emu_LoadCheatOption failed!\n");
    return -1;
}

int Emu_SaveCheatOption()
{
    return 0;
}
