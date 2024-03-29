#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <psp2/io/fcntl.h>

#include "list/cheat_list.h"
#include "list/option_list.h"
#include "list/overlay_list.h"
#include "activity/browser.h"
#include "setting/setting.h"
#include "emu/emu.h"
#include "config.h"
#include "file.h"
#include "utils.h"
#include "init.h"
#include "lang.h"

AppConfig app_config;
GraphicsConfig graphics_config;
ControlConfig control_config;
HotkeyConfig hotkey_config;
MiscConfig misc_config;

char *private_assets_dir = NULL;
char *public_assets_dir = NULL;

LinkedList *core_cheat_list = NULL;
LinkedList *core_option_list = NULL;
LinkedList *graphics_overlay_list = NULL;

void MakeConfigPath(char *path, char *config_name, int type)
{
    if (type == TYPE_CONFIG_GAME)
    {
        char name[MAX_NAME_LENGTH];
        MakeCurrentFileName(name);
        char base_name[MAX_NAME_LENGTH];
        MakeBaseName(base_name, name, MAX_NAME_LENGTH);
        snprintf(path, MAX_PATH_LENGTH, "%s/%s/%s", (CORE_CONFIGS_DIR), base_name, config_name);
    }
    else
    {
        snprintf(path, MAX_PATH_LENGTH, "%s/%s", (CORE_CONFIGS_DIR), config_name);
    }
}

int ResetGraphicsConfig()
{
    memset(&graphics_config, 0, sizeof(GraphicsConfig));
    graphics_config.version = GRAPHICS_CONFIG_VERSION;
    graphics_config.display_size = TYPE_DISPLAY_SIZE_FULL;
    graphics_config.aspect_ratio = TYPE_ASPECT_RATIO_DEFAULT;
    graphics_config.display_rotate = TYPE_DISPLAY_ROTATE_DEFAULT;
    graphics_config.graphics_shader = TYPE_GRAPHICS_SHADER_DEFAULT;
    graphics_config.graphics_smooth = 0;
    graphics_config.overlay_select = 0;
    graphics_config.overlay_mode = 0;
    graphics_config.show_fps = 0;

    return 0;
}

int ResetControlConfig()
{
    memset(&control_config, 0, sizeof(ControlConfig));
    control_config.version = CONTROL_CONFIG_VERSION;
    control_config.ctrl_player = 0;

    control_config.button_up = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_UP);
    control_config.button_down = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_DOWN);
    control_config.button_left = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_LEFT);
    control_config.button_right = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_RIGHT);
#if defined(FC_BUILD) || defined(GBC_BUILD) || defined(NGP_BUILD)
    control_config.button_circle = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_A);
    control_config.button_cross = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_B);
    control_config.button_triangle = ENABLE_TURBO_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_A);
    control_config.button_square = ENABLE_TURBO_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_B);
#if defined(FC_BUILD)
    control_config.button_l = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_L);
    control_config.button_r = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_R);
#endif
#elif defined(SFC_BUILD)
    control_config.button_circle = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_A);
    control_config.button_cross = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_B);
    control_config.button_triangle = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_X);
    control_config.button_square = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_Y);
    control_config.button_l = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_L);
    control_config.button_r = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_R);
#elif defined(GBA_BUILD)
    control_config.button_circle = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_A);
    control_config.button_cross = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_B);
    control_config.button_triangle = ENABLE_TURBO_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_A);
    control_config.button_square = ENABLE_TURBO_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_B);
    control_config.button_l = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_L);
    control_config.button_r = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_R);
#elif defined(MD_BUILD)
    control_config.button_circle = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_A);
    control_config.button_cross = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_B);
    control_config.button_triangle = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_X);
    control_config.button_square = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_Y);
    control_config.button_l = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_L);
    control_config.button_r = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_R);
#elif defined(WSC_BUILD)
    control_config.button_circle = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_A);
    control_config.button_cross = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_B);
    control_config.button_triangle = ENABLE_TURBO_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_A);
    control_config.button_square = ENABLE_TURBO_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_B);
    control_config.button_l = 0;
    control_config.button_r = 0;
#elif defined(PCE_BUILD)
    control_config.button_circle = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_A);
    control_config.button_cross = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_B);
    control_config.button_triangle = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_X);
    control_config.button_square = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_Y);
    control_config.button_l = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_L);
    control_config.button_r = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_R);
#elif defined(ARC_BUILD)
    control_config.button_circle = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_A);
    control_config.button_cross = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_B);
    control_config.button_triangle = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_X);
    control_config.button_square = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_Y);
    control_config.button_l = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_L);
    control_config.button_r = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_R);
    control_config.button_l2 = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_L2);
    control_config.button_r2 = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_R2);
#elif defined(PS_BUILD)
    control_config.button_circle = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_A);
    control_config.button_cross = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_B);
    control_config.button_triangle = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_X);
    control_config.button_square = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_Y);
    control_config.button_l = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_L);
    control_config.button_r = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_R);
    control_config.button_l2 = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_L2);
    control_config.button_r2 = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_R2);
    control_config.button_l3 = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_L3);
    control_config.button_r3 = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_R3);
#else
    control_config.button_circle = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_A);
    control_config.button_cross = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_B);
    control_config.button_triangle = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_X);
    control_config.button_square = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_Y);
    control_config.button_l = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_L);
    control_config.button_r = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_R);
    control_config.button_l2 = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_L2);
    control_config.button_r2 = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_R2);
    control_config.button_l3 = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_L3);
    control_config.button_r3 = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_R3);
#endif
#if !defined(WSC_BUILD)
    control_config.button_select = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_SELECT);
#endif
    control_config.button_start = ENABLE_BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_START);
    control_config.left_analog_up = control_config.button_up;
    control_config.left_analog_down = control_config.button_down;
    control_config.left_analog_left = control_config.button_left;
    control_config.left_analog_right = control_config.button_right;
#if defined(PS_BUILD)
    control_config.front_touch_pad = 1;
    control_config.back_touch_pad = 1;
#endif

    control_config.turbo_delay = 4;

    return 0;
}

int ResetHotkeyConfig()
{
    memset(&hotkey_config, 0, sizeof(HotkeyConfig));
    hotkey_config.version = HOTKEY_CONFIG_VERSION;
    hotkey_config.hk_loadstate = (SCE_CTRL_PSBUTTON | SCE_CTRL_SQUARE | ENABLE_KEY_BITMASK);
    hotkey_config.hk_savestate = (SCE_CTRL_PSBUTTON | SCE_CTRL_TRIANGLE | ENABLE_KEY_BITMASK);
    hotkey_config.hk_speed_up = (SCE_CTRL_PSBUTTON | SCE_CTRL_R1 | ENABLE_KEY_BITMASK);
    hotkey_config.hk_speed_down = (SCE_CTRL_PSBUTTON | SCE_CTRL_L1 | ENABLE_KEY_BITMASK);
    hotkey_config.hk_player_up = (SCE_CTRL_PSBUTTON | EXT_CTRL_RIGHT_ANLOG_RIGHT | ENABLE_KEY_BITMASK);
    hotkey_config.hk_player_down = (SCE_CTRL_PSBUTTON | EXT_CTRL_RIGHT_ANLOG_LEFT | ENABLE_KEY_BITMASK);
    hotkey_config.hk_exit_game = (SCE_CTRL_PSBUTTON | SCE_CTRL_CROSS | ENABLE_KEY_BITMASK);

    return 0;
}

int ResetMiscConfig()
{
    memset(&misc_config, 0, sizeof(MiscConfig));
    misc_config.version = MISC_CONFIG_VERSION;
    misc_config.auto_save_load = 1;

    return 0;
}

int ResetAppConfig()
{
    memset(&app_config, 0, sizeof(AppConfig));
    app_config.version = APP_CONFIG_VERSION;
    app_config.preview_path = TYPE_PREVIEW_PATH_AUTO;
    app_config.preview_style = TYPE_PREVIEW_SCALE_TYPE_FIT_CENTER_INSIDE;
    app_config.app_log = 1;
    app_config.core_log = 0;
#if defined(FBA_BUILD)
    app_config.show_log = 1;
#else
    app_config.show_log = 0;
#endif
    app_config.language = GetLangIndexByLocalLang(language);

    return 0;
}

int LoadGraphicsConfig(int type)
{
    GraphicsConfig config;
    memset(&config, 0, sizeof(GraphicsConfig));

    char path[MAX_PATH_LENGTH];
    MakeConfigPath(path, GRAPHICS_CONFIG_NAME, type);

    int ret = ReadFile(path, &config, sizeof(GraphicsConfig));
    if (ret < 0 || ret != sizeof(GraphicsConfig) || config.version != GRAPHICS_CONFIG_VERSION)
    {
        if (type == TYPE_CONFIG_MAIN)
            ResetGraphicsConfig();
        return -1;
    }

    memcpy(&graphics_config, &config, sizeof(GraphicsConfig));

    if (graphics_overlay_list && graphics_config.overlay_select > LinkedListGetLength(graphics_overlay_list))
        graphics_config.overlay_select = 0;

    return 0;
}

int LoadControlConfig(int type)
{
    ControlConfig config;
    memset(&config, 0, sizeof(ControlConfig));

    char path[MAX_PATH_LENGTH];
    MakeConfigPath(path, CONTROL_CONFIG_NAME, type);

    int ret = ReadFile(path, &config, sizeof(ControlConfig));
    if (ret < 0 || ret != sizeof(ControlConfig) || config.version != CONTROL_CONFIG_VERSION)
    {
        if (type == TYPE_CONFIG_MAIN)
            ResetControlConfig();
        return -1;
    }

    memcpy(&control_config, &config, sizeof(ControlConfig));

    return 0;
}

int LoadHotkeyConfig(int type)
{
    HotkeyConfig config;
    memset(&config, 0, sizeof(HotkeyConfig));

    char path[MAX_PATH_LENGTH];
    MakeConfigPath(path, HOTKEY_CONFIG_NAME, type);

    int ret = ReadFile(path, &config, sizeof(HotkeyConfig));
    if (ret < 0 || ret != sizeof(HotkeyConfig) || config.version != HOTKEY_CONFIG_VERSION)
    {
        if (type == TYPE_CONFIG_MAIN)
            ResetHotkeyConfig();
        return -1;
    }

    memcpy(&hotkey_config, &config, sizeof(HotkeyConfig));

    return 0;
}

int LoadMiscConfig(int type)
{
    MiscConfig config;
    memset(&config, 0, sizeof(MiscConfig));

    char path[MAX_PATH_LENGTH];
    MakeConfigPath(path, MISC_CONFIG_NAME, type);

    int ret = ReadFile(path, &config, sizeof(MiscConfig));
    if (ret < 0 || ret != sizeof(MiscConfig) || config.version != MISC_CONFIG_VERSION)
    {
        if (type == TYPE_CONFIG_MAIN)
            ResetMiscConfig();
        return -1;
    }

    memcpy(&misc_config, &config, sizeof(MiscConfig));

    return 0;
}

int LoadAppConfig(int type)
{
    AppConfig config;
    memset(&config, 0, sizeof(AppConfig));

    char path[MAX_PATH_LENGTH];
    MakeConfigPath(path, APP_CONFIG_NAME, type);

    int ret = ReadFile(path, &config, sizeof(AppConfig));
    if (ret < 0 || ret != sizeof(AppConfig) || config.version != APP_CONFIG_VERSION)
    {
        if (type == TYPE_CONFIG_MAIN)
            ResetAppConfig();
        return -1;
    }

    memcpy(&app_config, &config, sizeof(AppConfig));

    return 0;
}

int SaveGraphicsConfig(int type)
{
    char path[MAX_PATH_LENGTH];
    MakeConfigPath(path, GRAPHICS_CONFIG_NAME, type);

    char parent_path[MAX_PATH_LENGTH];
    MakeParentDir(parent_path, path, MAX_PATH_LENGTH);
    CreateFolder(parent_path);

    return WriteFile(path, &graphics_config, sizeof(GraphicsConfig));
}

int SaveControlConfig(int type)
{
    char path[MAX_PATH_LENGTH];
    MakeConfigPath(path, CONTROL_CONFIG_NAME, type);

    char parent_path[MAX_PATH_LENGTH];
    MakeParentDir(parent_path, path, MAX_PATH_LENGTH);
    CreateFolder(parent_path);

    return WriteFile(path, &control_config, sizeof(ControlConfig));
}

int SaveHotkeyConfig(int type)
{
    char path[MAX_PATH_LENGTH];
    MakeConfigPath(path, HOTKEY_CONFIG_NAME, type);

    char parent_path[MAX_PATH_LENGTH];
    MakeParentDir(parent_path, path, MAX_PATH_LENGTH);
    CreateFolder(parent_path);

    return WriteFile(path, &hotkey_config, sizeof(HotkeyConfig));
}

int SaveMiscConfig(int type)
{
    char path[MAX_PATH_LENGTH];
    MakeConfigPath(path, MISC_CONFIG_NAME, type);

    char parent_path[MAX_PATH_LENGTH];
    MakeParentDir(parent_path, path, MAX_PATH_LENGTH);
    CreateFolder(parent_path);

    return WriteFile(path, &misc_config, sizeof(MiscConfig));
}

int SaveAppConfig(int type)
{
    char path[MAX_PATH_LENGTH];
    MakeConfigPath(path, APP_CONFIG_NAME, type);

    char parent_path[MAX_PATH_LENGTH];
    MakeParentDir(parent_path, path, MAX_PATH_LENGTH);
    CreateFolder(parent_path);

    return WriteFile(path, &app_config, sizeof(AppConfig));
}

int ResetCoreConfig()
{
    if (!core_option_list)
        return -1;

    return OptionListResetConfig(core_option_list);
}

int LoadCoreConfig(int type)
{
    if (!core_option_list)
        return -1;

    char path[MAX_PATH_LENGTH];
    MakeConfigPath(path, CORE_CONFIG_NAME, type);

    if (!CheckFileExist(path))
    {
        if (type == TYPE_CONFIG_MAIN)
            ResetCoreConfig();
        return -1;
    }

    return OptionListLoadConfig(core_option_list, path);
}

int SaveCoreConfig(int type)
{
    if (!core_option_list)
        return -1;

    char path[MAX_PATH_LENGTH];
    MakeConfigPath(path, CORE_CONFIG_NAME, type);

    char parent_path[MAX_PATH_LENGTH];
    MakeParentDir(parent_path, path, MAX_PATH_LENGTH);
    CreateFolder(parent_path);

    return OptionListSaveConfig(core_option_list, path);
}
