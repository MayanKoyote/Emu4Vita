#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <psp2/appmgr.h>

#include "activity/activity.h"
#include "emu/emu.h"
#include "boot.h"
#include "utils.h"
#include "file.h"
#include "config.h"

#define BOOT_KEY_BOOT_MODE "boot_mode"
#define BOOT_KEY_GAME_PATH "game_path"
#define BOOT_KEY_PRIVATE_ASSETS_DIR "core_assets_dir"
#define BOOT_KEY_PUBLIC_ASSETS_DIR "common_assets_dir"
#define BOOT_KEY_RESTORE_APP_PATH "restore_app_path"

static int boot_mode = 0;
static char *boot_game_path = NULL;
static char *restore_app_path = NULL;
static int boot_argc = 0;
static char **boot_argv = NULL;

int BootGetMode()
{
    return boot_mode;
}

int BootLoadGame()
{
    if (!boot_game_path)
        return -1;

    Browser_ChangeDirByFilePath(boot_game_path);

    EmuGameInfo info;
    strcpy(info.path, boot_game_path);
    info.state_num = -2;
    Emu_StartGame(&info);

    return 0;
}

static int BootReadParam(const char *args)
{
    char *name = NULL;
    char *value = NULL;

    if (StringReadConfigLine(args, &name, &value) < 0)
        return -1;

    if (strcasecmp(name, BOOT_KEY_BOOT_MODE) == 0)
    {
        boot_mode = StringToDecimal(value);
    }
    else if (strcasecmp(name, BOOT_KEY_GAME_PATH) == 0)
    {
        boot_game_path = value;
        value = NULL;
    }
    else if (strcasecmp(name, BOOT_KEY_PRIVATE_ASSETS_DIR) == 0)
    {
        private_assets_dir = value;
        value = NULL;
    }
    else if (strcasecmp(name, BOOT_KEY_PUBLIC_ASSETS_DIR) == 0)
    {
        public_assets_dir = value;
        value = NULL;
    }
    else if (strcasecmp(name, BOOT_KEY_RESTORE_APP_PATH) == 0)
    {
        restore_app_path = value;
        value = NULL;
    }

    if (name)
        free(name);
    if (value)
        free(value);

    return 0;
}

int BootCheckParams(int argc, char *const *argv)
{
    if (argc < 2)
        goto END;

    APP_LOG("[BOOT] argc: %d\n", argc);
    boot_argc = argc;
    boot_argv = (char **)calloc(argc, sizeof(char *));

    int i, len;
    for (i = 1; i < argc; i++)
    {
        len = strlen(argv[i]);
        boot_argv[i - 1] = (char *)malloc(len + 1);
        strcpy(boot_argv[i - 1], argv[i]);
        BootReadParam(argv[i]);
        APP_LOG("[BOOT] argv[%d]: %s\n", i, argv[i]);
    }
    boot_argv[boot_argc - 1] = NULL;

END:
    if (!public_assets_dir)
    {
        public_assets_dir = (char *)malloc(MAX_PATH_LENGTH);
        strcpy(public_assets_dir, APP_ASSETS_DIR);
    }
    APP_LOG("[BOOT] Public assets dir: %s\n", public_assets_dir);

    return 0;
}

int BootLoadExec(const char *app_path, char *const *argv)
{
    APP_LOG("[BOOT] BootLoadExec: %s\n", app_path);
    return sceAppMgrLoadExec(app_path, argv, NULL);
}

int BootRestoreApp()
{
    if (boot_mode <= 0)
        return -1;

    const char *app_path = restore_app_path ? restore_app_path : "app0:eboot.bin";

    return BootLoadExec(app_path, boot_argv);
}

int BootLoadExecForGame(const char *app_path, const char *game_path, const char *assets_dir)
{
    if (!CheckFileExist(app_path))
        return -1;

    char boot_mode_param[MAX_CONFIG_LINE_LENGTH];
    snprintf(boot_mode_param, MAX_CONFIG_LINE_LENGTH, "%s=%d", BOOT_KEY_BOOT_MODE, BOOT_MODE_GAME);

    char game_path_param[MAX_CONFIG_LINE_LENGTH];
    snprintf(game_path_param, MAX_CONFIG_LINE_LENGTH, "%s=\"%s\"", BOOT_KEY_GAME_PATH, game_path);

    char private_assets_param[MAX_CONFIG_LINE_LENGTH];
    snprintf(private_assets_param, MAX_CONFIG_LINE_LENGTH, "%s=\"%s\"", BOOT_KEY_PRIVATE_ASSETS_DIR, assets_dir);

    char public_assets_param[MAX_CONFIG_LINE_LENGTH];
    snprintf(public_assets_param, MAX_CONFIG_LINE_LENGTH, "%s=\"%s\"", BOOT_KEY_PUBLIC_ASSETS_DIR, APP_ASSETS_DIR);

    char *argv[] = {
        boot_mode_param,
        game_path_param,
        private_assets_param,
        public_assets_param,
        NULL,
    };
    int ret = BootLoadExec(app_path, argv);

    return ret;
}

int BootLoadExecForCore(const char *app_path, const char *assets_dir)
{
    if (!CheckFileExist(app_path))
        return -1;

    char boot_mode_param[MAX_CONFIG_LINE_LENGTH];
    snprintf(boot_mode_param, MAX_CONFIG_LINE_LENGTH, "%s=%d", BOOT_KEY_BOOT_MODE, BOOT_MODE_ARCH);

    char private_assets_param[MAX_CONFIG_LINE_LENGTH];
    snprintf(private_assets_param, MAX_CONFIG_LINE_LENGTH, "%s=\"%s\"", BOOT_KEY_PRIVATE_ASSETS_DIR, assets_dir);

    char public_assets_param[MAX_CONFIG_LINE_LENGTH];
    snprintf(public_assets_param, MAX_CONFIG_LINE_LENGTH, "%s=\"%s\"", BOOT_KEY_PUBLIC_ASSETS_DIR, APP_ASSETS_DIR);

    char *argv[] = {
        boot_mode_param,
        private_assets_param,
        public_assets_param,
        NULL,
    };
    int ret = BootLoadExec(app_path, argv);

    return ret;
}
