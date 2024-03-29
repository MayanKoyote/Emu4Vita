#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "activity/browser_activity.h"
#include "emu/emu.h"
#include "config.h"

int CurrentPathIsGame()
{
    if (Emu_IsGameLoaded())
        return 1;

    return Browser_CurrentPathIsGame();
}

int GetCurrentRomType()
{
    if (Emu_IsGameLoaded())
        return Emu_GetCurrentRomType();

    return Browser_GetCurrentRomType();
}

int MakeCurrentGameName(char *name)
{
    if (Emu_IsGameLoaded())
        return Emu_MakeCurrentGameName(name);

    return Browser_MakeCurrentFileName(name);
}

int MakeCurrentGamePath(char *path)
{
    if (Emu_IsGameLoaded())
        return Emu_MakeCurrentGamePath(path);

    return Browser_MakeCurrentFilePath(path);
}

int MakeSavestateDir(char *path)
{
    char name[MAX_NAME_LENGTH];
    MakeCurrentGameName(name);
    char *p = strrchr(name, '.');
    if (p)
        *p = '\0';

    snprintf(path, MAX_PATH_LENGTH, "%s/%s", (CORE_SAVESTATES_DIR), name);
    return 0;
}

int MakeSavestatePath(char *path, int num)
{
    char dir[MAX_PATH_LENGTH];
    MakeSavestateDir(dir);

    if (num == -1)
        snprintf(path, MAX_PATH_LENGTH, "%s/state-auto.bin", dir);
    else
        snprintf(path, MAX_PATH_LENGTH, "%s/state-%02d.bin", dir, num);
    return 0;
}

int MakeSavefileDir(char *path)
{
    char name[MAX_NAME_LENGTH];
    MakeCurrentGameName(name);
    char *p = strrchr(name, '.');
    if (p)
        *p = '\0';

    snprintf(path, MAX_PATH_LENGTH, "%s/%s", (CORE_SAVEFILES_DIR), name);
    return 0;
}

int MakeSavefilePath(char *path, int id)
{
    char name[MAX_NAME_LENGTH];
    MakeCurrentGameName(name);
    char *p = strrchr(name, '.');
    if (p)
        *p = '\0';

    if (id == RETRO_MEMORY_SAVE_RAM)
        snprintf(path, MAX_PATH_LENGTH, "%s/%s.srm", (CORE_SAVEFILES_DIR), name);
    else if (id == RETRO_MEMORY_RTC)
        snprintf(path, MAX_PATH_LENGTH, "%s/%s.rtc", (CORE_SAVEFILES_DIR), name);
    else
        snprintf(path, MAX_PATH_LENGTH, "%s/%s.unk", (CORE_SAVEFILES_DIR), name);
    return 0;
}

int MakeCheatPath(char *path)
{
    char name[MAX_NAME_LENGTH];
    MakeCurrentGameName(name);
    char *p = strrchr(name, '.');
    if (p)
        *p = '\0';

    snprintf(path, MAX_PATH_LENGTH, "%s/%s.cht", (CORE_CHEATS_DIR), name);
    return 0;
}

int MakeCheatPath2(char *path)
{
    MakeCurrentGamePath(path);
    char *p = strrchr(path, '.');
    if (!p++)
        p = path + strlen(path);

    strcpy(p, "cht");
    return 0;
}

int MakePreviewPath(char *path, char *ext)
{
    char cur_path[MAX_PATH_LENGTH];
    char parent_dir[MAX_PATH_LENGTH];
    char base_name[MAX_NAME_LENGTH];
    MakeCurrentGamePath(cur_path);
    MakeParentDir(parent_dir, cur_path, MAX_PATH_LENGTH);
    MakeBaseName(base_name, cur_path, MAX_NAME_LENGTH);

    snprintf(path, MAX_PATH_LENGTH, "%s%s/%s.%s", parent_dir, PREVIEW_DIR_NAME, base_name, ext);
    return 0;
}

int MakeScreenshotPath(char *path)
{
    char name[MAX_NAME_LENGTH];
    MakeCurrentGameName(name);
    char *p = strrchr(name, '.');
    if (p)
        *p = '\0';

    int i;
    for (i = 0; i < 1000; i++)
    {
        snprintf(path, MAX_PATH_LENGTH, "%s/%s_%d.png", CORE_SCREENSHOTS_DIR, name, i);
        if (!CheckFileExist(path))
            return 1;
    }

    return 0;
}

int MakeSplashPath(char *path)
{
    char name[MAX_NAME_LENGTH];
    MakeCurrentGameName(name);
    char *p = strrchr(name, '.');
    if (p)
        *p = '\0';

    snprintf(path, MAX_PATH_LENGTH, "%s/%s.png", (CORE_SPLASHS_DIR), name);
    return 0;
}
