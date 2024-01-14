#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <psp2/appmgr.h>

#include "boot.h"
#include "file.h"
#include "config.h"

#define EXEC_BOOT_MODE_KEY_STR "boot_mode"
#define BOOT_GAME_PATH_KEY_STR "game_path"
#define PRIVATE_ASSETS_KEY_STR "core_assets_dir"
#define PUBLIC_ASSETS_KEY_STR "common_assets_dir"
#define RESTORE_APP_PATH_KEY_STR "restore_app_path"

int BootLoadExec(char *app_path, char *argv[])
{
    int ret = sceAppMgrLoadExec(app_path, (char *const *)argv, NULL);

    return ret;
}

int BootLoadExecForGame(char *app_path, char *game_path, char *assets_dir)
{
    if (!checkFileExist(app_path))
        return -1;

    char boot_mode_param[MAX_CONFIG_LINE_LENGTH];
    snprintf(boot_mode_param, MAX_CONFIG_LINE_LENGTH, "%s=%d", EXEC_BOOT_MODE_KEY_STR, BOOT_MODE_GAME);

    char game_path_param[MAX_CONFIG_LINE_LENGTH];
    snprintf(game_path_param, MAX_CONFIG_LINE_LENGTH, "%s=\"%s\"", BOOT_GAME_PATH_KEY_STR, game_path);

    char private_assets_param[MAX_CONFIG_LINE_LENGTH];
    snprintf(private_assets_param, MAX_CONFIG_LINE_LENGTH, "%s=\"%s/%s\"", PRIVATE_ASSETS_KEY_STR, CORE_DATA_DIR, assets_dir);

    char public_assets_param[MAX_CONFIG_LINE_LENGTH];
    snprintf(public_assets_param, MAX_CONFIG_LINE_LENGTH, "%s=\"%s\"", PUBLIC_ASSETS_KEY_STR, APP_ASSETS_DIR);

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

int BootLoadExecForCore(char *app_path, char *assets_dir)
{
    if (!checkFileExist(app_path))
        return -1;

    char boot_mode_param[MAX_CONFIG_LINE_LENGTH];
    snprintf(boot_mode_param, MAX_CONFIG_LINE_LENGTH, "%s=%d", EXEC_BOOT_MODE_KEY_STR, BOOT_MODE_ARCH);

    char private_assets_param[MAX_CONFIG_LINE_LENGTH];
    snprintf(private_assets_param, MAX_CONFIG_LINE_LENGTH, "%s=\"%s/%s\"", PRIVATE_ASSETS_KEY_STR, CORE_DATA_DIR, assets_dir);

    char public_assets_param[MAX_CONFIG_LINE_LENGTH];
    snprintf(public_assets_param, MAX_CONFIG_LINE_LENGTH, "%s=\"%s\"", PUBLIC_ASSETS_KEY_STR, APP_ASSETS_DIR);

    char *argv[] = {
        boot_mode_param,
        private_assets_param,
        public_assets_param,
        NULL,
    };
    int ret = BootLoadExec(app_path, argv);

    return ret;
}
