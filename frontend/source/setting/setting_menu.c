#include <string.h>
#include <stdint.h>

#include "gui/gui.h"
#include "emu/emu.h"
#include "setting_types.h"
#include "setting_menu.h"
#include "setting_option.h"
#include "setting_callbacks.h"
#include "lang.h"
#include "config.h"

extern int setting_current_path_is_file;
extern int setting_game_is_loaded;

extern int setting_visibility_core_menu;
extern int setting_visibility_cheat_menu;
extern int setting_visibility_app_menu;
extern int setting_visibility_disk_control_item;
extern int setting_visibility_exit_to_arch_item;
extern int setting_visibility_touch_to_button_item;

extern uint32_t setting_language_config_value;

static LangString no_yes_names[] = {
    {LANG_NO, NULL},
    {LANG_YES, NULL},
};

static LangString display_size_names[] = {
    {LANG_DISPLAY_SIZE_1X, NULL},
    {LANG_DISPLAY_SIZE_2X, NULL},
    {LANG_DISPLAY_SIZE_3X, NULL},
    {LANG_DISPLAY_SIZE_FULL, NULL},
};

static LangString display_ratio_names[] = {
    {LANG_DEFAULT, NULL},
    {LANG_ASPECT_RATIO_BY_GAME_RESOLUTION, NULL},
    {LANG_ASPECT_RATIO_BY_DEV_SCREEN, NULL},
    {LANG_ASPECT_RATIO_8_7, NULL},
    {LANG_ASPECT_RATIO_4_3, NULL},
    {LANG_ASPECT_RATIO_3_2, NULL},
    {LANG_ASPECT_RATIO_16_9, NULL},
};

#if defined(WANT_DISPLAY_ROTATE)
static LangString display_rotate_names[] = {
    {LANG_DISABLE, NULL},
    {LANG_DISPLAY_ROTATE_CW_90, NULL},
    {LANG_DISPLAY_ROTATE_CW_180, NULL},
    {LANG_DISPLAY_ROTATE_CW_270, NULL},
    {LANG_DEFAULT, NULL},
};
#endif

static LangString graphics_shader_names[] = {
    {LANG_DEFAULT, NULL},
    {LANG_SHADER_LCD3X, NULL},
    {LANG_SHADER_SHARP_BILINEAR_SIMPLE, NULL},
    {LANG_SHADER_SHARP_BILINEAR, NULL},
    {LANG_SHADER_ADVANCED_AA, NULL},
};

static LangString overlay_mode_names[] = {
    {LANG_OVERLAY_MODE_OVERLAY, NULL},
    {LANG_OVERLAY_MODE_BACKGROUND, NULL},
};

static LangString ctrl_player_names[] = {
    {LANG_NULL, "1P"},
    {LANG_NULL, "2P"},
    {LANG_NULL, "3P"},
    {LANG_NULL, "4P"},
};

static LangString preview_path_names[] = {
    {LANG_AUTO, NULL},
    {LANG_DEFAULT, NULL},
    {LANG_PREVIEW_PATH_FROM_AUTO_STATE, NULL},
};

static LangString preview_style_names[] = {
    {LANG_PREVIEW_STYLE_PRESERVE_FULL, NULL},
    {LANG_PREVIEW_STYLE_STRETCH_FULL, NULL},
    {LANG_PREVIEW_STYLE_CROP_FULL, NULL},
};

static KeyMapEntry emu_keymap_entries[] = {
    {{LANG_TURBO, NULL}, TURBO_BITMASK_KEY},
#if defined(WSC_BUILD)
    {{LANG_BUTTON_X1, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_LEFT)},
    {{LANG_BUTTON_X2, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_UP)},
    {{LANG_BUTTON_X3, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_RIGHT)},
    {{LANG_BUTTON_X4, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_DOWN)},
#else
    {{LANG_BUTTON_LEFT, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_LEFT)},
    {{LANG_BUTTON_UP, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_UP)},
    {{LANG_BUTTON_RIGHT, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_RIGHT)},
    {{LANG_BUTTON_DOWN, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_DOWN)},
#endif
#if defined(FC_BUILD) || defined(GBC_BUILD) || defined(NGP_BUILD)
    {{LANG_BUTTON_A, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_A)},
    {{LANG_BUTTON_B, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_B)},
    {{LANG_BUTTON_SELECT, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_SELECT)},
    {{LANG_BUTTON_START, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_START)},
#if defined(FC_BUILD)
    {{LANG_FDS_DISK_SIDE_CHANGE, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_L)},
    {{LANG_FDS_INSERT_EJECT_DISK, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_R)},
#endif
#elif defined(GBA_BUILD)
    {{LANG_BUTTON_A, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_A)},
    {{LANG_BUTTON_B, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_B)},
    {{LANG_BUTTON_L, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_L)},
    {{LANG_BUTTON_R, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_R)},
    {{LANG_BUTTON_SELECT, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_SELECT)},
    {{LANG_BUTTON_START, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_START)},
#elif defined(SFC_BUILD)
    {{LANG_BUTTON_A, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_A)},
    {{LANG_BUTTON_B, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_B)},
    {{LANG_BUTTON_X, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_X)},
    {{LANG_BUTTON_Y, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_Y)},
    {{LANG_BUTTON_L, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_L)},
    {{LANG_BUTTON_R, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_R)},
    {{LANG_BUTTON_SELECT, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_SELECT)},
    {{LANG_BUTTON_START, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_START)},
#elif defined(MD_BUILD)
    {{LANG_BUTTON_A, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_Y)},
    {{LANG_BUTTON_B, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_B)},
    {{LANG_BUTTON_C, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_A)},
    {{LANG_BUTTON_X, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_L)},
    {{LANG_BUTTON_Y, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_X)},
    {{LANG_BUTTON_Z, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_R)},
    {{LANG_SWICTH_MODE, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_SELECT)},
    {{LANG_BUTTON_START, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_START)},
#elif defined(WSC_BUILD)
    {{LANG_BUTTON_Y1, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_L)},
    {{LANG_BUTTON_Y2, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_R2)},
    {{LANG_BUTTON_Y3, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_R)},
    {{LANG_BUTTON_Y4, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_L2)},
    {{LANG_BUTTON_A, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_A)},
    {{LANG_BUTTON_B, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_B)},
    {{LANG_BUTTON_START, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_START)},
#elif defined(PCE_BUILD)
    {{LANG_BUTTON_A, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_A)},
    {{LANG_BUTTON_B, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_B)},
    {{LANG_BUTTON_X, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_X)},
    {{LANG_BUTTON_Y, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_Y)},
    {{LANG_BUTTON_L, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_L)},
    {{LANG_BUTTON_R, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_R)},
    {{LANG_BUTTON_SELECT, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_SELECT)},
    {{LANG_BUTTON_START, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_START)},
    {{LANG_SWICTH_MODE, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_L2)},
#elif defined(FBA_BUILD)
    {{LANG_BUTTON_A, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_B)},
    {{LANG_BUTTON_B, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_A)},
    {{LANG_BUTTON_C, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_Y)},
    {{LANG_BUTTON_D, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_X)},
    {{LANG_BUTTON_E, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_L)},
    {{LANG_BUTTON_F, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_R)},
    {{LANG_BUTTON_G, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_L2)},
    {{LANG_BUTTON_H, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_R2)},
    {{LANG_COIN, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_SELECT)},
    {{LANG_START, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_START)},
#elif defined(PS_BUILD)
    {{LANG_BUTTON_CROSS, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_B)},
    {{LANG_BUTTON_CIRCLE, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_A)},
    {{LANG_BUTTON_SQUARE, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_Y)},
    {{LANG_BUTTON_TRIANGLE, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_X)},
    {{LANG_BUTTON_L, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_L)},
    {{LANG_BUTTON_R, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_R)},
    {{LANG_BUTTON_L2, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_L2)},
    {{LANG_BUTTON_R2, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_R2)},
    {{LANG_BUTTON_L3, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_L3)},
    {{LANG_BUTTON_R3, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_R3)},
    {{LANG_BUTTON_SELECT, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_SELECT)},
    {{LANG_BUTTON_START, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_START)},
#else
    {{LANG_BUTTON_A, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_A)},
    {{LANG_BUTTON_B, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_B)},
    {{LANG_BUTTON_X, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_X)},
    {{LANG_BUTTON_Y, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_Y)},
    {{LANG_BUTTON_L, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_L)},
    {{LANG_BUTTON_R, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_R)},
    {{LANG_BUTTON_L2, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_L2)},
    {{LANG_BUTTON_R2, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_R2)},
    {{LANG_BUTTON_L3, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_L3)},
    {{LANG_BUTTON_R3, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_R3)},
    {{LANG_BUTTON_SELECT, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_SELECT)},
    {{LANG_BUTTON_START, NULL}, RETRO_BITMASK_KEY(RETRO_DEVICE_ID_JOYPAD_START)},
#endif
};

static KeyMapEntry hotkey_keymap_entries[] = {
    {{LANG_TURBO, NULL}, TURBO_BITMASK_KEY},
    {{LANG_BUTTON_PSBUTTON, NULL}, SCE_CTRL_PSBUTTON},
    {{LANG_BUTTON_LEFT, NULL}, SCE_CTRL_LEFT},
    {{LANG_BUTTON_UP, NULL}, SCE_CTRL_UP},
    {{LANG_BUTTON_RIGHT, NULL}, SCE_CTRL_RIGHT},
    {{LANG_BUTTON_DOWN, NULL}, SCE_CTRL_DOWN},
    {{LANG_BUTTON_CROSS, NULL}, SCE_CTRL_CROSS},
    {{LANG_BUTTON_CIRCLE, NULL}, SCE_CTRL_CIRCLE},
    {{LANG_BUTTON_SQUARE, NULL}, SCE_CTRL_SQUARE},
    {{LANG_BUTTON_TRIANGLE, NULL}, SCE_CTRL_TRIANGLE},
    {{LANG_BUTTON_L, NULL}, SCE_CTRL_L1},
    {{LANG_BUTTON_R, NULL}, SCE_CTRL_R1},
    {{LANG_BUTTON_L2, NULL}, SCE_CTRL_L2},
    {{LANG_BUTTON_R2, NULL}, SCE_CTRL_R2},
    {{LANG_BUTTON_L3, NULL}, SCE_CTRL_L3},
    {{LANG_BUTTON_R3, NULL}, SCE_CTRL_R3},
    {{LANG_BUTTON_SELECT, NULL}, SCE_CTRL_SELECT},
    {{LANG_BUTTON_START, NULL}, SCE_CTRL_START},
    {{LANG_BUTTON_LEFT_ANALOG_LEFT, NULL}, EXT_CTRL_LEFT_ANLOG_LEFT},
    {{LANG_BUTTON_LEFT_ANALOG_UP, NULL}, EXT_CTRL_LEFT_ANLOG_UP},
    {{LANG_BUTTON_LEFT_ANALOG_RIGHT, NULL}, EXT_CTRL_LEFT_ANLOG_RIGHT},
    {{LANG_BUTTON_LEFT_ANALOG_DOWN, NULL}, EXT_CTRL_LEFT_ANLOG_DOWN},
    {{LANG_BUTTON_RIGHT_ANALOG_LEFT, NULL}, EXT_CTRL_RIGHT_ANLOG_LEFT},
    {{LANG_BUTTON_RIGHT_ANALOG_UP, NULL}, EXT_CTRL_RIGHT_ANLOG_UP},
    {{LANG_BUTTON_RIGHT_ANALOG_RIGHT, NULL}, EXT_CTRL_RIGHT_ANLOG_RIGHT},
    {{LANG_BUTTON_RIGHT_ANALOG_DOWN, NULL}, EXT_CTRL_RIGHT_ANLOG_DOWN},
};

// 图形 (设置选项)
static StrArrayOption display_size_option = {&graphics_config.display_size, display_size_names, sizeof(display_size_names) / sizeof(LangString)};
static StrArrayOption aspect_ratio_option = {&graphics_config.aspect_ratio, display_ratio_names, sizeof(display_ratio_names) / sizeof(LangString)};
#if defined(WANT_DISPLAY_ROTATE)
static StrArrayOption display_rotate_option = {&graphics_config.display_rotate, display_rotate_names, sizeof(display_rotate_names) / sizeof(LangString)};
#endif
static StrArrayOption graphics_shader_option = {&graphics_config.graphics_shader, graphics_shader_names, sizeof(graphics_shader_names) / sizeof(LangString)};
static StrArrayOption graphics_smooth_option = {&graphics_config.graphics_smooth, no_yes_names, sizeof(no_yes_names) / sizeof(LangString)};
static StrArrayOption overlay_select_option = {&graphics_config.overlay_select, NULL, 0};
static StrArrayOption overlay_mode_option = {&graphics_config.overlay_mode, overlay_mode_names, sizeof(overlay_mode_names) / sizeof(LangString)};
static StrArrayOption show_fps_option = {&graphics_config.show_fps, no_yes_names, sizeof(no_yes_names) / sizeof(LangString)};

// 控制 (设置选项)
static StrArrayOption ctrl_player_option = {&control_config.ctrl_player, ctrl_player_names, sizeof(ctrl_player_names) / sizeof(LangString)};
static KeyMapOption button_left_option = {&control_config.button_left, emu_keymap_entries, sizeof(emu_keymap_entries) / sizeof(KeyMapEntry)};
static KeyMapOption button_up_option = {&control_config.button_up, emu_keymap_entries, sizeof(emu_keymap_entries) / sizeof(KeyMapEntry)};
static KeyMapOption button_right_option = {&control_config.button_right, emu_keymap_entries, sizeof(emu_keymap_entries) / sizeof(KeyMapEntry)};
static KeyMapOption button_down_option = {&control_config.button_down, emu_keymap_entries, sizeof(emu_keymap_entries) / sizeof(KeyMapEntry)};
static KeyMapOption button_cross_option = {&control_config.button_cross, emu_keymap_entries, sizeof(emu_keymap_entries) / sizeof(KeyMapEntry)};
static KeyMapOption button_circle_option = {&control_config.button_circle, emu_keymap_entries, sizeof(emu_keymap_entries) / sizeof(KeyMapEntry)};
static KeyMapOption button_square_option = {&control_config.button_square, emu_keymap_entries, sizeof(emu_keymap_entries) / sizeof(KeyMapEntry)};
static KeyMapOption button_triangle_option = {&control_config.button_triangle, emu_keymap_entries, sizeof(emu_keymap_entries) / sizeof(KeyMapEntry)};
static KeyMapOption button_l_option = {&control_config.button_l, emu_keymap_entries, sizeof(emu_keymap_entries) / sizeof(KeyMapEntry)};
static KeyMapOption button_r_option = {&control_config.button_r, emu_keymap_entries, sizeof(emu_keymap_entries) / sizeof(KeyMapEntry)};
static KeyMapOption button_l2_option = {&control_config.button_l2, emu_keymap_entries, sizeof(emu_keymap_entries) / sizeof(KeyMapEntry)};
static KeyMapOption button_r2_option = {&control_config.button_r2, emu_keymap_entries, sizeof(emu_keymap_entries) / sizeof(KeyMapEntry)};
static KeyMapOption button_l3_option = {&control_config.button_l3, emu_keymap_entries, sizeof(emu_keymap_entries) / sizeof(KeyMapEntry)};
static KeyMapOption button_r3_option = {&control_config.button_r3, emu_keymap_entries, sizeof(emu_keymap_entries) / sizeof(KeyMapEntry)};
static KeyMapOption button_select_option = {&control_config.button_select, emu_keymap_entries, sizeof(emu_keymap_entries) / sizeof(KeyMapEntry)};
static KeyMapOption button_start_option = {&control_config.button_start, emu_keymap_entries, sizeof(emu_keymap_entries) / sizeof(KeyMapEntry)};
static KeyMapOption left_analog_left_option = {&control_config.left_analog_left, emu_keymap_entries, sizeof(emu_keymap_entries) / sizeof(KeyMapEntry)};
static KeyMapOption left_analog_up_option = {&control_config.left_analog_up, emu_keymap_entries, sizeof(emu_keymap_entries) / sizeof(KeyMapEntry)};
static KeyMapOption left_analog_right_option = {&control_config.left_analog_right, emu_keymap_entries, sizeof(emu_keymap_entries) / sizeof(KeyMapEntry)};
static KeyMapOption left_analog_down_option = {&control_config.left_analog_down, emu_keymap_entries, sizeof(emu_keymap_entries) / sizeof(KeyMapEntry)};
static KeyMapOption right_analog_left_option = {&control_config.right_analog_left, emu_keymap_entries, sizeof(emu_keymap_entries) / sizeof(KeyMapEntry)};
static KeyMapOption right_analog_up_option = {&control_config.right_analog_up, emu_keymap_entries, sizeof(emu_keymap_entries) / sizeof(KeyMapEntry)};
static KeyMapOption right_analog_right_option = {&control_config.right_analog_right, emu_keymap_entries, sizeof(emu_keymap_entries) / sizeof(KeyMapEntry)};
static KeyMapOption right_analog_down_option = {&control_config.right_analog_down, emu_keymap_entries, sizeof(emu_keymap_entries) / sizeof(KeyMapEntry)};
static StrArrayOption front_touch_pad_option = {&control_config.front_touch_pad, no_yes_names, sizeof(no_yes_names) / sizeof(LangString)};
static StrArrayOption back_touch_pad_option = {&control_config.back_touch_pad, no_yes_names, sizeof(no_yes_names) / sizeof(LangString)};
static IntRangeOption turbo_delay_option = {&control_config.turbo_delay, 1, 30, 1, "%d"};

// 热键 (设置选项)
static KeyMapOption hk_savestate_option = {&hotkey_config.hk_savestate, hotkey_keymap_entries, sizeof(hotkey_keymap_entries) / sizeof(KeyMapEntry)};
static KeyMapOption hk_loadstate_option = {&hotkey_config.hk_loadstate, hotkey_keymap_entries, sizeof(hotkey_keymap_entries) / sizeof(KeyMapEntry)};
static KeyMapOption hk_speed_up_option = {&hotkey_config.hk_speed_up, hotkey_keymap_entries, sizeof(hotkey_keymap_entries) / sizeof(KeyMapEntry)};
static KeyMapOption hk_speed_down_option = {&hotkey_config.hk_speed_down, hotkey_keymap_entries, sizeof(hotkey_keymap_entries) / sizeof(KeyMapEntry)};
static KeyMapOption hk_rewind_game_option = {&hotkey_config.hk_rewind_game, hotkey_keymap_entries, sizeof(hotkey_keymap_entries) / sizeof(KeyMapEntry)};
static KeyMapOption hk_player_up_option = {&hotkey_config.hk_player_up, hotkey_keymap_entries, sizeof(hotkey_keymap_entries) / sizeof(KeyMapEntry)};
static KeyMapOption hk_player_down_option = {&hotkey_config.hk_player_down, hotkey_keymap_entries, sizeof(hotkey_keymap_entries) / sizeof(KeyMapEntry)};
static KeyMapOption hk_exit_game_option = {&hotkey_config.hk_exit_game, hotkey_keymap_entries, sizeof(hotkey_keymap_entries) / sizeof(KeyMapEntry)};

// 杂项 (设置选项)
static StrArrayOption auto_save_load_option = {&misc_config.auto_save_load, no_yes_names, sizeof(no_yes_names) / sizeof(LangString)};
static StrArrayOption rewind_enabled_option = {&misc_config.enable_rewind, no_yes_names, sizeof(no_yes_names) / sizeof(LangString)};
static IntRangeOption rewind_max_count_option = {&misc_config.rewind_max_count, 10, 1000, 10, "%d"};
static IntRangeOption rewind_interval_time_option = {&misc_config.rewind_interval_time, 1, 100, 1, "%d"};

// 程序 (设置选项)
static StrArrayOption preview_path_option = {&app_config.preview_path, preview_path_names, sizeof(preview_path_names) / sizeof(LangString)};
static StrArrayOption preview_style_option = {&app_config.preview_style, preview_style_names, sizeof(preview_style_names) / sizeof(LangString)};
static StrArrayOption app_log_option = {&app_config.app_log, no_yes_names, sizeof(no_yes_names) / sizeof(LangString)};
static StrArrayOption core_log_option = {&app_config.core_log, no_yes_names, sizeof(no_yes_names) / sizeof(LangString)};
static StrArrayOption show_log_option = {&app_config.show_log, no_yes_names, sizeof(no_yes_names) / sizeof(LangString)};
static StrArrayOption language_option = {&setting_language_config_value, NULL, 0};

// 主菜单 (菜单条目)
static SettingMenuItem main_menu_items[] = {
    FUCNTION_ITEM(LANG_RESUME_GAME, &setting_game_is_loaded, Setting_onResumeGameItemClick),
    FUCNTION_ITEM(LANG_RESET_GAME, &setting_game_is_loaded, Setting_onResetGameItemClick),
    FUCNTION_ITEM(LANG_EXIT_GAME, &setting_game_is_loaded, Setting_onExitGameItemClick),
    FUCNTION_ITEM(LANG_DISK_CONTROL, &setting_visibility_disk_control_item, Setting_onDiskControlItemClick),
    FUCNTION_ITEM(LANG_EXIT_TO_ARCH, &setting_visibility_exit_to_arch_item, Setting_onExitToArchItemClick),
    FUCNTION_ITEM(LANG_EXIT_APP, NULL, Setting_onExitAppItemClick),
};

// 图形 (菜单条目)
static SettingMenuItem graphics_menu_items[] = {
    STRARRAY_OPTION_ITEM(LANG_DISPLAY_SIZE, &display_size_option, NULL, Setting_onGraphicsMenuOptionChanged),
    STRARRAY_OPTION_ITEM(LANG_ASPECT_RATIO, &aspect_ratio_option, NULL, Setting_onGraphicsMenuOptionChanged),
#if defined(WANT_DISPLAY_ROTATE)
    STRARRAY_OPTION_ITEM(LANG_DISPLAY_ROTATE, &display_rotate_option, NULL, Setting_onGraphicsMenuOptionChanged),
#endif
    STRARRAY_OPTION_ITEM(LANG_GRAPHICS_SHADER, &graphics_shader_option, NULL, Setting_onGraphicsMenuOptionChanged),
    STRARRAY_OPTION_ITEM(LANG_GRAPHICS_SMOOTH, &graphics_smooth_option, NULL, Setting_onGraphicsMenuOptionChanged),
    STRARRAY_OPTION_ITEM(LANG_OVERLAY_SELECT, &overlay_select_option, NULL, Setting_onGraphicsMenuOptionChanged),
    STRARRAY_OPTION_ITEM(LANG_OVERLAY_MODE, &overlay_mode_option, NULL, Setting_onGraphicsMenuOptionChanged),
    STRARRAY_OPTION_ITEM(LANG_SHOW_FPS, &show_fps_option, NULL, Setting_onGraphicsMenuOptionChanged),
    FUCNTION_ITEM(LANG_RESET_CONFIGS, NULL, Setting_onResetGraphicsConfigItemClick),
};

// 控制 (菜单条目)
static SettingMenuItem control_menu_items[] = {
    STRARRAY_OPTION_ITEM(LANG_CTRL_PLAYER, &ctrl_player_option, NULL, NULL),

    KEYMAP_OPTION_ITEM(LANG_BUTTON_LEFT, &button_left_option, NULL, NULL),
    KEYMAP_OPTION_ITEM(LANG_BUTTON_UP, &button_up_option, NULL, NULL),
    KEYMAP_OPTION_ITEM(LANG_BUTTON_RIGHT, &button_right_option, NULL, NULL),
    KEYMAP_OPTION_ITEM(LANG_BUTTON_DOWN, &button_down_option, NULL, NULL),
    KEYMAP_OPTION_ITEM(LANG_BUTTON_CROSS, &button_cross_option, NULL, NULL),
    KEYMAP_OPTION_ITEM(LANG_BUTTON_CIRCLE, &button_circle_option, NULL, NULL),
    KEYMAP_OPTION_ITEM(LANG_BUTTON_SQUARE, &button_square_option, NULL, NULL),
    KEYMAP_OPTION_ITEM(LANG_BUTTON_TRIANGLE, &button_triangle_option, NULL, NULL),
    KEYMAP_OPTION_ITEM(LANG_BUTTON_L, &button_l_option, NULL, NULL),
    KEYMAP_OPTION_ITEM(LANG_BUTTON_R, &button_r_option, NULL, NULL),
    KEYMAP_OPTION_ITEM(LANG_BUTTON_L2, &button_l2_option, NULL, NULL),
    KEYMAP_OPTION_ITEM(LANG_BUTTON_R2, &button_r2_option, NULL, NULL),
    KEYMAP_OPTION_ITEM(LANG_BUTTON_L3, &button_l3_option, NULL, NULL),
    KEYMAP_OPTION_ITEM(LANG_BUTTON_R3, &button_r3_option, NULL, NULL),
    KEYMAP_OPTION_ITEM(LANG_BUTTON_SELECT, &button_select_option, NULL, NULL),
    KEYMAP_OPTION_ITEM(LANG_BUTTON_START, &button_start_option, NULL, NULL),
    KEYMAP_OPTION_ITEM(LANG_BUTTON_LEFT_ANALOG_UP, &left_analog_up_option, NULL, NULL),
    KEYMAP_OPTION_ITEM(LANG_BUTTON_LEFT_ANALOG_LEFT, &left_analog_left_option, NULL, NULL),
    KEYMAP_OPTION_ITEM(LANG_BUTTON_LEFT_ANALOG_RIGHT, &left_analog_right_option, NULL, NULL),
    KEYMAP_OPTION_ITEM(LANG_BUTTON_LEFT_ANALOG_DOWN, &left_analog_down_option, NULL, NULL),
    KEYMAP_OPTION_ITEM(LANG_BUTTON_RIGHT_ANALOG_LEFT, &right_analog_left_option, NULL, NULL),
    KEYMAP_OPTION_ITEM(LANG_BUTTON_RIGHT_ANALOG_UP, &right_analog_up_option, NULL, NULL),
    KEYMAP_OPTION_ITEM(LANG_BUTTON_RIGHT_ANALOG_RIGHT, &right_analog_right_option, NULL, NULL),
    KEYMAP_OPTION_ITEM(LANG_BUTTON_RIGHT_ANALOG_DOWN, &right_analog_down_option, NULL, NULL),

    STRARRAY_OPTION_ITEM(LANG_FRONT_TOUCH_TO_BUTTON, &front_touch_pad_option, &setting_visibility_touch_to_button_item, NULL),
    STRARRAY_OPTION_ITEM(LANG_BACK_TOUCH_TO_BUTTON, &back_touch_pad_option, &setting_visibility_touch_to_button_item, NULL),
    INTRANGE_OPTION_ITEM(LANG_TURBO_DELAY, &turbo_delay_option, NULL, NULL),
    FUCNTION_ITEM(LANG_RESET_CONFIGS, NULL, Setting_onResetControlConfigItemClick),
};

// 热键 (菜单条目)
static SettingMenuItem hotkey_menu_items[] = {
    KEYMAP_OPTION_ITEM(LANG_HOTKEY_SAVESTATE, &hk_savestate_option, NULL, NULL),
    KEYMAP_OPTION_ITEM(LANG_HOTKEY_LOADSTATE, &hk_loadstate_option, NULL, NULL),
    KEYMAP_OPTION_ITEM(LANG_HOTKEY_GAME_SPEED_UP, &hk_speed_up_option, NULL, NULL),
    KEYMAP_OPTION_ITEM(LANG_HOTKEY_GAME_SPEED_DOWN, &hk_speed_down_option, NULL, NULL),
    KEYMAP_OPTION_ITEM(LANG_HOTKEY_GAME_REWIND, &hk_rewind_game_option, NULL, NULL),
    KEYMAP_OPTION_ITEM(LANG_HOTKEY_CONTROL_PLAYER_UP, &hk_player_up_option, NULL, NULL),
    KEYMAP_OPTION_ITEM(LANG_HOTKEY_CONTROL_PLAYER_DOWN, &hk_player_down_option, NULL, NULL),
    KEYMAP_OPTION_ITEM(LANG_HOTKEY_EXIT_GAME, &hk_exit_game_option, NULL, NULL),
    FUCNTION_ITEM(LANG_RESET_CONFIGS, NULL, Setting_onResetHotkeyConfigItemClick),
};

// 杂项 (菜单条目)
static SettingMenuItem misc_menu_items[] = {
    STRARRAY_OPTION_ITEM(LANG_AUTO_SAVE_LOAD_STATE, &auto_save_load_option, NULL, Setting_onAutoSaveLoadOptionChanged),
    STRARRAY_OPTION_ITEM(LANG_ENABLE_REWIND, &rewind_enabled_option, NULL, NULL),
    INTRANGE_OPTION_ITEM(LANG_REWIND_MAX_COUNT, &rewind_max_count_option, NULL, NULL),
    INTRANGE_OPTION_ITEM(LANG_REWIND_INTERVAL_TIME, &rewind_interval_time_option, NULL, NULL),
    FUCNTION_ITEM(LANG_SAVE_SCREENSHOT, &setting_game_is_loaded, Setting_onSaveScreenshotItemClick),
    FUCNTION_ITEM(LANG_SAVE_PREVIEW, &setting_game_is_loaded, Setting_onSavePreviewItemClick),
    FUCNTION_ITEM(LANG_RESET_CONFIGS, NULL, Setting_onResetMiscConfigItemClick),
};

// 程序 (菜单条目)
static SettingMenuItem app_menu_items[] = {
    STRARRAY_OPTION_ITEM(LANG_PREVIEW_PATH, &preview_path_option, NULL, Setting_onPreviewOptionChanged),
    STRARRAY_OPTION_ITEM(LANG_PREVIEW_STYLE, &preview_style_option, NULL, Setting_onPreviewOptionChanged),
    STRARRAY_OPTION_ITEM(LANG_APP_LOG, &app_log_option, NULL, NULL),
    STRARRAY_OPTION_ITEM(LANG_CORE_LOG, &core_log_option, NULL, NULL),
    STRARRAY_OPTION_ITEM(LANG_SHOW_LOG, &show_log_option, NULL, NULL),
    STRARRAY_OPTION_ITEM(LANG_LANGUAGE, &language_option, NULL, Setting_onLanguageOptionChanged),
    FUCNTION_ITEM(LANG_RESET_CONFIGS, NULL, Setting_onResetAppConfigItemClick),
};

// 菜单列表
static SettingMenu setting_menus[] = {
    SETTING_MENU(LANG_MENU_MAIN, main_menu_items, sizeof(main_menu_items) / sizeof(SettingMenuItem), NULL, NULL, NULL),                                                            // 主菜单                                                                                 // 主菜单
    SETTING_MENU(LANG_MENU_STATE, NULL, 0, &setting_current_path_is_file, Setting_onStateMenuStart, Setting_onStateMenuFinish),                                                    // 即时存档
    SETTING_MENU(LANG_MENU_GRAPHICS, graphics_menu_items, sizeof(graphics_menu_items) / sizeof(SettingMenuItem), NULL, Setting_onGraphicsMenuStart, Setting_onGraphicsMenuFinish), // 图形
    SETTING_MENU(LANG_MENU_CONTROL, control_menu_items, sizeof(control_menu_items) / sizeof(SettingMenuItem), NULL, Setting_onControlMenuStart, Setting_onControlMenuFinish),      // 控制
    SETTING_MENU(LANG_MENU_HOTKEY, hotkey_menu_items, sizeof(hotkey_menu_items) / sizeof(SettingMenuItem), NULL, Setting_onHotkeyMenuStart, Setting_onHotkeyMenuFinish),           // 热键
    SETTING_MENU(LANG_MENU_CORE, NULL, 0, &setting_visibility_core_menu, Setting_onCoreMenuStart, Setting_onCoreMenuFinish),                                                       // 核心
    SETTING_MENU(LANG_MENU_CHEAT, NULL, 0, &setting_visibility_cheat_menu, Setting_onCheatMenuStart, Setting_onCheatMenuFinish),                                                   // 金手指
    SETTING_MENU(LANG_MENU_MISC, misc_menu_items, sizeof(misc_menu_items) / sizeof(SettingMenuItem), NULL, Setting_onMiscMenuStart, Setting_onMiscMenuFinish),                     // 杂项
    SETTING_MENU(LANG_MENU_APP, app_menu_items, sizeof(app_menu_items) / sizeof(SettingMenuItem), &setting_visibility_app_menu, Setting_onAppMenuStart, Setting_onAppMenuFinish),  // 程序
};

SettingContext setting_context = {
    setting_menus,                               // menus
    sizeof(setting_menus) / sizeof(SettingMenu), // n_menus
    0,                                           // menus_pos
};

SettingContext *Setting_GetContext()
{
    return &setting_context;
}

StrArrayOption *Setting_GetLangOption()
{
    return &language_option;
}

StrArrayOption *Setting_GetOverlaySelectOption()
{
    return &overlay_select_option;
}
