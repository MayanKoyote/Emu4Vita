#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <psp2/io/fcntl.h>

#include "list/config_list.h"
#include "list/cheat_list.h"
#include "list/option_list.h"
#include "list/overlay_list.h"
#include "activity/activity.h"
#include "setting/setting.h"
#include "emu/emu.h"
#include "gui/gui.h"
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
    graphics_config.display_size = DEFAULT_CONFIG_VALUE_GRAPHICS_DISPLAY_SIZE;
    graphics_config.aspect_ratio = DEFAULT_CONFIG_VALUE_GRAPHICS_DISPLAY_RATIO;
    graphics_config.display_rotate = DEFAULT_CONFIG_VALUE_GRAPHICS_DISPLAY_ROTATE;
    graphics_config.graphics_shader = DEFAULT_CONFIG_VALUE_GRAPHICS_GRAPHICS_SHADER;
    graphics_config.graphics_smooth = DEFAULT_CONFIG_VALUE_GRAPHICS_GRAPHICS_SMOOTH;
    graphics_config.overlay_select = DEFAULT_CONFIG_VALUE_GRAPHICS_OVERLAY_SELECT;
    graphics_config.overlay_mode = DEFAULT_CONFIG_VALUE_GRAPHICS_OVERLAY_MODE;
    graphics_config.show_fps = DEFAULT_CONFIG_VALUE_GRAPHICS_SHOW_FPS;

    return 0;
}

int ResetControlConfig()
{
    memset(&control_config, 0, sizeof(ControlConfig));
    control_config.version = CONTROL_CONFIG_VERSION;
    control_config.ctrl_player = DEFAULT_CONFIG_VALUE_CONTROL_CTRL_PLAYER;
    control_config.button_up = DEFAULT_CONFIG_VALUE_CONTROL_BUTTON_UP;
    control_config.button_down = DEFAULT_CONFIG_VALUE_CONTROL_BUTTON_DOWN;
    control_config.button_left = DEFAULT_CONFIG_VALUE_CONTROL_BUTTON_LEFT;
    control_config.button_right = DEFAULT_CONFIG_VALUE_CONTROL_BUTTON_RIGHT;
    control_config.button_circle = DEFAULT_CONFIG_VALUE_CONTROL_BUTTON_CIRCLE;
    control_config.button_cross = DEFAULT_CONFIG_VALUE_CONTROL_BUTTON_CROSS;
    control_config.button_triangle = DEFAULT_CONFIG_VALUE_CONTROL_BUTTON_TRIANGLE;
    control_config.button_square = DEFAULT_CONFIG_VALUE_CONTROL_BUTTON_SQUARE;
    control_config.button_l = DEFAULT_CONFIG_VALUE_CONTROL_BUTTON_L;
    control_config.button_r = DEFAULT_CONFIG_VALUE_CONTROL_BUTTON_R;
    control_config.button_l2 = DEFAULT_CONFIG_VALUE_CONTROL_BUTTON_L2;
    control_config.button_r2 = DEFAULT_CONFIG_VALUE_CONTROL_BUTTON_R2;
    control_config.button_l3 = DEFAULT_CONFIG_VALUE_CONTROL_BUTTON_L3;
    control_config.button_r3 = DEFAULT_CONFIG_VALUE_CONTROL_BUTTON_R3;
    control_config.button_select = DEFAULT_CONFIG_VALUE_CONTROL_BUTTON_SELECT;
    control_config.button_start = DEFAULT_CONFIG_VALUE_CONTROL_BUTTON_START;
    control_config.left_analog_up = DEFAULT_CONFIG_VALUE_CONTROL_BUTTON_LEFT_ANALOG_UP;
    control_config.left_analog_down = DEFAULT_CONFIG_VALUE_CONTROL_BUTTON_LEFT_ANALOG_DOWN;
    control_config.left_analog_left = DEFAULT_CONFIG_VALUE_CONTROL_BUTTON_LEFT_ANALOG_LEFT;
    control_config.left_analog_right = DEFAULT_CONFIG_VALUE_CONTROL_BUTTON_LEFT_ANALOG_RIGHT;
    control_config.right_analog_up = DEFAULT_CONFIG_VALUE_CONTROL_BUTTON_RIGHT_ANALOG_UP;
    control_config.right_analog_down = DEFAULT_CONFIG_VALUE_CONTROL_BUTTON_RIGHT_ANALOG_DOWN;
    control_config.right_analog_left = DEFAULT_CONFIG_VALUE_CONTROL_BUTTON_RIGHT_ANALOG_LEFT;
    control_config.right_analog_right = DEFAULT_CONFIG_VALUE_CONTROL_BUTTON_RIGHT_ANALOG_RIGHT;
    control_config.front_touch_pad = DEFAULT_CONFIG_VALUE_CONTROL_FRONT_TOUCH_PAD;
    control_config.back_touch_pad = DEFAULT_CONFIG_VALUE_CONTROL_BACK_TOUCH_PAD;
    control_config.turbo_delay = DEFAULT_CONFIG_VALUE_CONTROL_TURBO_DELEY;

    return 0;
}

int ResetHotkeyConfig()
{
    memset(&hotkey_config, 0, sizeof(HotkeyConfig));
    hotkey_config.version = HOTKEY_CONFIG_VERSION;
    hotkey_config.hk_savestate = DEFAULT_CONFIG_VALUE_HOTKEY_SAVE_STATE;
    hotkey_config.hk_loadstate = DEFAULT_CONFIG_VALUE_HOTKEY_LOAD_STATE;
    hotkey_config.hk_speed_up = DEFAULT_CONFIG_VALUE_HOTKEY_SPEED_UP;
    hotkey_config.hk_speed_down = DEFAULT_CONFIG_VALUE_HOTKEY_SPEED_DOWN;
    hotkey_config.hk_rewind_game = DEFAULT_CONFIG_VALUE_HOTKEY_REWIND_GAME;
    hotkey_config.hk_player_up = DEFAULT_CONFIG_VALUE_HOTKEY_PLAYER_UP;
    hotkey_config.hk_player_down = DEFAULT_CONFIG_VALUE_HOTKEY_PLAYER_DOWN;
    hotkey_config.hk_exit_game = DEFAULT_CONFIG_VALUE_HOTKEY_EXIT_GAME;

    return 0;
}

int ResetMiscConfig()
{
    memset(&misc_config, 0, sizeof(MiscConfig));
    misc_config.version = MISC_CONFIG_VERSION;
    misc_config.auto_save_load = DEFAULT_CONFIG_VALUE_MISC_AUTO_SAVE_LOAD;
    misc_config.enable_rewind = DEFAULT_CONFIG_VALUE_MISC_ENABLE_REWIND;
    misc_config.rewind_max_count = DEFAULT_CONFIG_VALUE_MISC_REWIND_MAX_COUNT;
    misc_config.rewind_interval_time = DEFAULT_CONFIG_VALUE_MISC_REWIND_INTERVAL_TIME;

    return 0;
}

int ResetAppConfig()
{
    memset(&app_config, 0, sizeof(AppConfig));
    app_config.version = APP_CONFIG_VERSION;
    app_config.preview_path = DEFAULT_CONFIG_VALUE_APP_PREVIEW_PATH;
    app_config.preview_style = DEFAULT_CONFIG_VALUE_APP_PREVIEW_SCALE_TYPE;
    app_config.app_log = DEFAULT_CONFIG_VALUE_APP_APP_LOG;
    app_config.core_log = DEFAULT_CONFIG_VALUE_APP_CORE_LOG;
    app_config.show_log = DEFAULT_CONFIG_VALUE_APP_SHOW_LOG;
    app_config.language = GetLangIndexByLocalLang(language); // DEFAULT_CONFIG_VALUE_APP_LANGUAGE

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
