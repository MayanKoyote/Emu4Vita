#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <psp2/appmgr.h>

#include "boot.h"
#include "file.h"
#include "config.h"

#define STR_KEY_BOOT_MODE "boot_mode"
#define STR_KEY_BOOT_GAME_PATH "game_path"
#define STR_KEY_PRIVATE_ASSETS_DIR "core_assets_dir"
#define STR_KEY_PUBLIC_ASSETS_DIR "common_assets_dir"
#define STR_KEY_RESTORE_APP_PATH "restore_app_path"

int BootLoadExec(const char *app_path, char *const *argv)
{
    return sceAppMgrLoadExec(app_path, argv, NULL);
}

int BootLoadExecForGame(const char *app_path, const char *game_path, const char *assets_dir)
{
    if (!CheckFileExist(app_path))
        return -1;

    char boot_mode_param[MAX_CONFIG_LINE_LENGTH];
    snprintf(boot_mode_param, MAX_CONFIG_LINE_LENGTH, "%s=%d", STR_KEY_BOOT_MODE, BOOT_MODE_GAME);

    char game_path_param[MAX_CONFIG_LINE_LENGTH];
    snprintf(game_path_param, MAX_CONFIG_LINE_LENGTH, "%s=\"%s\"", STR_KEY_BOOT_GAME_PATH, game_path);

    char private_assets_param[MAX_CONFIG_LINE_LENGTH];
    snprintf(private_assets_param, MAX_CONFIG_LINE_LENGTH, "%s=\"%s\"", STR_KEY_PRIVATE_ASSETS_DIR, assets_dir);

    char public_assets_param[MAX_CONFIG_LINE_LENGTH];
    snprintf(public_assets_param, MAX_CONFIG_LINE_LENGTH, "%s=\"%s\"", STR_KEY_PUBLIC_ASSETS_DIR, APP_ASSETS_DIR);

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
    snprintf(boot_mode_param, MAX_CONFIG_LINE_LENGTH, "%s=%d", STR_KEY_BOOT_MODE, BOOT_MODE_ARCH);

    char private_assets_param[MAX_CONFIG_LINE_LENGTH];
    snprintf(private_assets_param, MAX_CONFIG_LINE_LENGTH, "%s=\"%s\"", STR_KEY_PRIVATE_ASSETS_DIR, assets_dir);

    char public_assets_param[MAX_CONFIG_LINE_LENGTH];
    snprintf(public_assets_param, MAX_CONFIG_LINE_LENGTH, "%s=\"%s\"", STR_KEY_PUBLIC_ASSETS_DIR, APP_ASSETS_DIR);

    char *argv[] = {
        boot_mode_param,
        private_assets_param,
        public_assets_param,
        NULL,
    };
    int ret = BootLoadExec(app_path, argv);

    return ret;
}
