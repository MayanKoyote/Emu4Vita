#ifndef __M_LANG_INTL_H__
#define __M_LANG_INTL_H__

#include "lang.h"

/* English */
char *lang_us[LANGUAGE_CONTAINER_SIZE] = {
    // LANG_NULL
    NULL,

    /**************  General  *****************/
    // LANG_YES
    "Yes",
    // LANG_NO
    "No",
    // LANG_CONFIRM
    "Confirm",
    // LANG_CANCEL
    "Cancel",
    // LANG_BACK
    "Back",
    // LANG_EXIT
    "Exit",

    /**************  Alert dialog  *****************/
    // LANG_TIP
    "Tip",
    // LANG_MENU
    "Menu",
    // LANG_COLSE
    "Close",

    /**************  Safe mode  *****************/
    // LANG_MESSAGE_WARN_SAFE_MODE
    "Currently in safe mode, please enable unsafe homebrew in HENkaku settings first, "
    "only then can this program be used normally.",

    /**************  Button string  *****************/
    // LANG_BUTTON_LEFT
    "←",
    // LANG_BUTTON_UP
    "↑",
    // LANG_BUTTON_RIGHT
    "→",
    // LANG_BUTTON_DOWN
    "↓",
    // LANG_BUTTON_A
    "A",
    // LANG_BUTTON_B
    "B",
    // LANG_BUTTON_X
    "X",
    // LANG_BUTTON_Y
    "Y",
    // LANG_BUTTON_L
    "L",
    // LANG_BUTTON_R
    "R",
    // LANG_BUTTON_L1
    "L1",
    // LANG_BUTTON_R1
    "R1",
    // LANG_BUTTON_L2
    "L2",
    // LANG_BUTTON_R2
    "R2",
    // LANG_BUTTON_L3
    "L3",
    // LANG_BUTTON_R3
    "R3",
    // LANG_BUTTON_SELECT
    "Select",
    // LANG_BUTTON_START
    "Start",
    // LANG_BUTTON_HOME
    "Home",
    // LANG_BUTTON_LEFT_ANALOG_LEFT
    "Left analog ←",
    // LANG_BUTTON_LEFT_ANALOG_UP
    "Left analog ↑",
    // LANG_BUTTON_LEFT_ANALOG_RIGHT
    "Left analog →",
    // LANG_BUTTON_LEFT_ANALOG_DOWN
    "Left analog ↓",
    // LANG_BUTTON_RIGHT_ANALOG_LEFT
    "Right analog ←",
    // LANG_BUTTON_RIGHT_ANALOG_UP
    "Right analog ↑",
    // LANG_BUTTON_RIGHT_ANALOG_RIGHT
    "Right analog →",
    // LANG_BUTTON_RIGHT_ANALOG_DOWN
    "Right analog ↓",

    // LANG_BUTTON_ENTER
    NULL,
    // LANG_BUTTON_CANCEL
    NULL,

    /**************  Button string 2  *****************/
    // LANG_BUTTON_CROSS
    "╳",
    // LANG_BUTTON_CIRCLE
    "○",
    // LANG_BUTTON_SQUARE
    "□",
    // LANG_BUTTON_TRIANGLE
    "△",

    // LANG_BUTTON_C
    "C",
    // LANG_BUTTON_D
    "D",
    // LANG_BUTTON_E
    "E",
    // LANG_BUTTON_F
    "F",
    // LANG_BUTTON_G
    "G",
    // LANG_BUTTON_H
    "H",
    // LANG_BUTTON_Z
    "Z",

    // LANG_BUTTON_X1
    "X1",
    // LANG_BUTTON_X2
    "X2",
    // LANG_BUTTON_X3
    "X3",
    // LANG_BUTTON_X4
    "X4",
    // LANG_BUTTON_Y1
    "Y1",
    // LANG_BUTTON_Y2
    "Y2",
    // LANG_BUTTON_Y3
    "Y3",
    // LANG_BUTTON_Y4
    "Y4",

    // LANG_SWICTH_MODE
    "Swicth mode",
    // LANG_COIN
    "Coin",
    // LANG_SELECT
    "Select",
    // LANG_START
    "Start",
    // LANG_TURBO
    "Turbo",
    // LANG_FDS_DISK_SIDE_CHANGE
    "[FDS] Disk side change",
    // LANG_FDS_INSERT_EJECT_DISK
    "[FDS] Insert/Eject disk",

    /**************  Browser  *****************/
    // LANG_APP_TITLE
    APP_NAME " v" APP_VER,

    // LANG_PARENT_DIR
    "Parent dir",
    // LANG_OPEN_DIR
    "Open dir",
    // LANG_START_GAME
    "Start game",
    // LANG_OPTION_MENU
    "Option menu",
    // LANG_SETTING_MENU
    "Setting menu",
    // LANG_ABOUT
    "About",
    // LANG_CHANGE_DIR
    "Change dir",

    // LANG_OPTION_MENU_START_GAME
    "Start game",
    // LANG_OPTION_MENU_DELETE_GAME
    "Delete game",
    // LANG_OPTION_MENU_DELETE_AUTO_SAVESTATE
    "Delete auto savestate",
    // LANG_OPTION_MENU_DELETE_AUTO_SAVEFILE
    "Delete auto savefile",
    // LANG_OPTION_MENU_DELETE_CACHE_FILES
    "Delete cache files",

    // LANG_MESSAGE_ASK_DELETE_GAME
    "Are you sure you want to delete this game?",
    // LANG_MESSAGE_ASK_DELETE_AUTO_STATE
    "Are you sure you want to delete the auto savestate of this game?",
    // LANG_MESSAGE_ASK_DELETE_AUTO_SAVEFILE
    "Are you sure you want to delete the auto savefile of this game?",
    // LANG_MESSAGE_ASK_DELETE_CACHE_FILES
    "Are you sure you want to delete the cache files of this game?",
    // LANG_MESSAGE_START_GAME_FAILED
    "Failed to start this game!",
    // LANG_MESSAGE_WAIT_EXITING
    "Exiting, please wait......",

    /**************  About  *****************/
    // LANG_ABOUT_TITLE
    "About",

    /**************  Menu table  *****************/
    // LANG_MENU_MAIN
    "Main",
    // LANG_MENU_STATE
    "State",
    // LANG_MENU_GRAPHICS
    "Graphics",
    // LANG_MENU_CONTROL
    "Control",
    // LANG_MENU_HOTKEY
    "Hotkey",
    // LANG_MENU_CORE
    "Core",
    // LANG_MENU_CHEAT
    "Cheat",
    // LANG_MENU_MISC
    "Misc",
    // LANG_MENU_APP
    "App",

    /**************  Menu general  *****************/
    // LANG_DISABLE
    "Disable",
    // LANG_DEFAULT
    "Default",
    // LANG_AUTO
    "Auto",
    // LANG_NONE
    "None",
    // LANG_RESET_CONFIGS
    "Reset configs",

    /**************  Menu main  *****************/
    // LANG_RESUME_GAME
    "Resume game",
    // LANG_RESET_GAME
    "Reset game",
    // LANG_EXIT_GAME
    "Exit game",
    // LANG_DISK_CONTROL
    "Disk control",
    // LANG_EXIT_TO_ARCH
    "Eixt to Arch",
    // LANG_EXIT_APP
    "Exit app",

    // LANG_SWITCH_DISK
    "Switch disk",
    // LANG_DISK
    "Disk",
    // LANG_CURRENT
    "Current",

    /**************  Menu graphics  *****************/
    // LANG_DISPLAY_SIZE
    "Display size",
    // LANG_ASPECT_RATIO
    "Aspect ratio",
    // LANG_DISPLAY_ROTATE
    "Display rotate",
    // LANG_GRAPHICS_SHADER
    "Graphics shader",
    // LANG_GRAPHICS_SMOOTH
    "Graphics smooth",
    // LANG_OVERLAY_SELECT
    "Overlay select",
    // LANG_OVERLAY_MODE
    "Overlay mode",
    // LANG_SHOW_FPS
    "Show FPS",

    // LANG_DISPLAY_SIZE_1X
    "1X",
    // LANG_DISPLAY_SIZE_2X
    "2X",
    // LANG_DISPLAY_SIZE_3X
    "3X",
    // LANG_DISPLAY_SIZE_FULL
    "Full",

    // LANG_ASPECT_RATIO_BY_GAME_RESOLUTION
    "By game resolution",
    // LANG_ASPECT_RATIO_BY_DEV_SCREEN
    "By device screen",
    // LANG_ASPECT_RATIO_8_7
    "8:7",
    // LANG_ASPECT_RATIO_4_3
    "4:3",
    // LANG_ASPECT_RATIO_3_2
    "3:2",
    // LANG_ASPECT_RATIO_16_9
    "16:9",

    // LANG_DISPLAY_ROTATE_CW_90
    "Rotate 90°CW",
    // LANG_DISPLAY_ROTATE_CW_180
    "Rotate 180°CW",
    // LANG_DISPLAY_ROTATE_CW_270
    "Rotate 270°CW",

    // LANG_SHADER_LCD3X
    "LCD3x",
    // LANG_SHADER_SHARP_BILINEAR_SIMPLE
    "Sharp bilinear simple",
    // LANG_SHADER_SHARP_BILINEAR
    "Sharp bilinear",
    // LANG_SHADER_ADVANCED_AA
    "Advanced AA",

    // LANG_OVERLAY_MODE_OVERLAY
    "Overlay",
    // LANG_OVERLAY_MODE_BACKGROUND
    "Background",

    /**************  Menu control  *****************/
    // LANG_CONTROLLER_PORT
    "Controller port",
    // LANG_FRONT_TOUCH_TO_BUTTON
    "Front touch to button",
    // LANG_BACK_TOUCH_TO_BUTTON
    "Back touch to button",
    // LANG_TURBO_DELAY
    "Turbo delay (frames)",

    /**************  Menu misc  *****************/
    // LANG_AUTO_SAVE_LOAD_STATE
    "Auto save/load state",
    // LANG_ENABLE_REWIND
    "Enable game rewind",
    // LANG_REWIND_MAX_COUNT
    "Max rewind count",
    // LANG_REWIND_INTERVAL_TIME
    "Rewind interval time (seconds)",
    // LANG_SAVE_SCREENSHOT
    "Save screenshot",
    // LANG_SAVE_PREVIEW
    "Save screenshot for preview",

    // LANG_SAVE_SCREENSHOT_OK,
    "Save screenshot OK!",
    // LANG_SAVE_SCREENSHOT_FAILED,
    "Failed to save screenshot!",
    // LANG_SAVE_PREVIEW_OK,
    "Save screenshot for preview OK!",
    // LANG_SAVE_PREVIEW_FAILED,
    "Failed to save screenshot for preview!",

    /**************  Menu hotkey  *****************/
    // LANG_HOTKEY_SAVESTATE
    "[Hotkey] Save state",
    // LANG_HOTKEY_LOADSTATE
    "[Hotkey] Load state",
    // LANG_HOTKEY_GAME_SPEED_UP
    "[Hotkey] Game speed up",
    // LANG_HOTKEY_GAME_SPEED_DOWN
    "[Hotkey] Game speed down",
    // LANG_HOTKEY_GAME_REWIND
    "[Hotkey] Game rewind",
    // LANG_HOTKEY_CONTROLLER_PORT_UP
    "[Hotkey] Controller port +",
    // LANG_HOTKEY_CONTROLLER_PORT_DOWN
    "[Hotkey] Controller port -",
    // LANG_HOTKEY_EXIT_GAME
    "[Hotkey] Exit game",

    /**************  Menu app  *****************/
    // LANG_PREVIEW_PATH
    "Preview path",
    // LANG_PREVIEW_STYLE
    "Preview style",
    // LANG_APP_LOG
    "App log",
    // LANG_CORE_LOG
    "Core log",
    // LANG_SHOW_LOG
    "Show log (loading)",
    // LANG_LANGUAGE
    "Language",

    // LANG_PREVIEW_PATH_FROM_AUTO_STATE
    "From auto savestate",

    // LANG_PREVIEW_STYLE_PRESERVE_FULL
    "Preserve full",
    // LANG_PREVIEW_STYLE_STRETCH_FULL
    "Stretch full",
    // LANG_PREVIEW_STYLE_CROP_FULL,
    "Crop full",

    /**************  Menu state  *****************/
    // LANG_STATE_EXISTENT_STATE
    "State",
    // LANG_STATE_EMPTY_STATE
    "Empty",

    // LANG_STATE_LOAD_STATE
    "Load",
    // LANG_STATE_SAVE_STATE
    "Save",
    // LANG_STATE_DELETE_STATE
    "Delete",
};

/* 简体中文 */
char *lang_chs[LANGUAGE_CONTAINER_SIZE] = {
    // LANG_NULL
    NULL,

    /**************  General  *****************/
    // LANG_YES
    "是",
    // LANG_NO
    "否",
    // LANG_CONFIRM
    "确定",
    // LANG_CANCEL
    "取消",
    // LANG_BACK
    "返回",
    // LANG_EXIT
    "退出",

    /**************  Dialog  *****************/
    // LANG_TIP
    "提示",
    // LANG_MENU
    "菜单",
    // LANG_COLSE
    "关闭",

    /**************  Safe mode  *****************/
    // LANG_MESSAGE_WARN_SAFE_MODE
    "当前处于安全模式，请先在HENkaku设置里开启启用不安全自制软件，"
    "然后才能正常使用本程序。",

    /**************  Button string  *****************/
    // LANG_BUTTON_LEFT
    "←",
    // LANG_BUTTON_UP
    "↑",
    // LANG_BUTTON_RIGHT
    "→",
    // LANG_BUTTON_DOWN
    "↓",
    // LANG_BUTTON_A
    "A",
    // LANG_BUTTON_B
    "B",
    // LANG_BUTTON_X
    "X",
    // LANG_BUTTON_Y
    "Y",
    // LANG_BUTTON_L
    "L",
    // LANG_BUTTON_R
    "R",
    // LANG_BUTTON_L1
    "L1",
    // LANG_BUTTON_R1
    "R1",
    // LANG_BUTTON_L2
    "L2",
    // LANG_BUTTON_R2
    "R2",
    // LANG_BUTTON_L3
    "L3",
    // LANG_BUTTON_R3
    "R3",
    // LANG_BUTTON_SELECT
    "Select",
    // LANG_BUTTON_START
    "Start",
    // LANG_BUTTON_HOME
    "Home",
    // LANG_BUTTON_LEFT_ANALOG_LEFT
    "左摇杆←",
    // LANG_BUTTON_LEFT_ANALOG_UP
    "左摇杆↑",
    // LANG_BUTTON_LEFT_ANALOG_RIGHT
    "左摇杆→",
    // LANG_BUTTON_LEFT_ANALOG_DOWN
    "左摇杆↓",
    // LANG_BUTTON_RIGHT_ANALOG_LEFT
    "右摇杆←",
    // LANG_BUTTON_RIGHT_ANALOG_UP
    "右摇杆↑",
    // LANG_BUTTON_RIGHT_ANALOG_RIGHT
    "右摇杆→",
    // LANG_BUTTON_RIGHT_ANALOG_DOWN
    "右摇杆↓",

    // LANG_BUTTON_ENTER
    NULL,
    // LANG_BUTTON_CANCEL
    NULL,

    /**************  Button string 2  *****************/
    // LANG_BUTTON_CROSS
    "╳",
    // LANG_BUTTON_CIRCLE
    "○",
    // LANG_BUTTON_SQUARE
    "□",
    // LANG_BUTTON_TRIANGLE
    "△",

    // LANG_BUTTON_C
    "C",
    // LANG_BUTTON_D
    "D",
    // LANG_BUTTON_E
    "E",
    // LANG_BUTTON_F
    "F",
    // LANG_BUTTON_G
    "G",
    // LANG_BUTTON_H
    "H",
    // LANG_BUTTON_Z
    "Z",

    // LANG_BUTTON_X1
    "X1",
    // LANG_BUTTON_X2
    "X2",
    // LANG_BUTTON_X3
    "X3",
    // LANG_BUTTON_X4
    "X4",
    // LANG_BUTTON_Y1
    "Y1",
    // LANG_BUTTON_Y2
    "Y2",
    // LANG_BUTTON_Y3
    "Y3",
    // LANG_BUTTON_Y4
    "Y4",

    // LANG_SWICTH_MODE
    "模式切换",
    // LANG_COIN
    "投币",
    // LANG_SELECT
    "选择",
    // LANG_START
    "开始",
    // LANG_TURBO
    "连发",
    // LANG_FDS_DISK_SIDE_CHANGE
    "[FDS] 更换磁盘面",
    // LANG_FDS_INSERT_EJECT_DISK
    "[FDS] 插入/弹出磁盘",

    /**************  Browser  *****************/
    // LANG_APP_TITLE
    APP_NAME " v" APP_VER,

    // LANG_PARENT_DIR
    "上层目录",
    // LANG_OPEN_DIR
    "打开目录",
    // LANG_START_GAME
    "启动游戏",
    // LANG_OPTION_MENU
    "选项菜单",
    // LANG_SETTING_MENU
    "设置菜单",
    // LANG_ABOUT
    "关于",
    // LANG_CHANGE_DIR
    "跳转目录",

    // LANG_OPTION_MENU_START_GAME
    "启动游戏",
    // LANG_OPTION_MENU_DELETE_GAME
    "删除游戏",
    // LANG_OPTION_MENU_DELETE_AUTO_SAVESTATE
    "删除自动即时存档",
    // LANG_OPTION_MENU_DELETE_AUTO_SAVEFILE
    "删除自动模拟存档",
    // LANG_OPTION_MENU_DELETE_CACHE_FILES
    "删除缓存文件",

    // LANG_MESSAGE_ASK_DELETE_GAME
    "确认要删除这个游戏？",
    // LANG_MESSAGE_ASK_DELETE_AUTO_STATE
    "确认要删除这个游戏的自动即时存档？",
    // LANG_MESSAGE_ASK_DELETE_AUTO_SAVEFILE
    "确认要删除这个游戏的自动模拟存档？",
    // LANG_MESSAGE_ASK_DELETE_CACHE_FILES
    "确认要删除这个游戏的缓存文件？",
    // LANG_MESSAGE_START_GAME_FAILED
    "启动游戏失败！",
    // LANG_MESSAGE_WAIT_EXITING
    "正在退出，请稍候......",

    /**************  About  *****************/
    // LANG_ABOUT_TITLE
    "关于",

    /**************  Menu tab  *****************/
    // LANG_MENU_MAIN
    "主菜单",
    // LANG_MENU_STATE
    "即时存档",
    // LANG_MENU_GRAPHICS
    "图形",
    // LANG_MENU_CONTROL
    "控制",
    // LANG_MENU_HOTKEY
    "快捷键",
    // LANG_MENU_CORE
    "核心",
    // LANG_MENU_CHEAT
    "金手指",
    // LANG_MENU_MISC
    "杂项",
    // LANG_MENU_APP
    "程序",

    /**************  Menu general  *****************/
    // LANG_DISABLE
    "禁用",
    // LANG_DEFAULT
    "默认",
    // LANG_AUTO
    "自动",
    // LANG_NONE
    "无",
    // LANG_RESET_CONFIGS
    "恢复默认设置",

    /**************  Menu main  *****************/
    // LANG_RESUME_GAME
    "继续游戏",
    // LANG_RESET_GAME
    "重置游戏",
    // LANG_EXIT_GAME
    "退出游戏",
    // LANG_DISK_CONTROL
    "光盘控制",
    // LANG_EXIT_TO_ARCH
    "返回前端",
    // LANG_EXIT_APP
    "退出程序",

    // LANG_SWITCH_DISK
    "更换光盘",
    // LANG_DISK
    "光盘",
    // LANG_CURRENT
    "当前",

    /**************  Menu graphics  *****************/
    // LANG_DISPLAY_SIZE
    "画面尺寸",
    // LANG_ASPECT_RATIO
    "画面比例",
    // LANG_DISPLAY_ROTATE
    "画面旋转",
    // LANG_GRAPHICS_SHADER
    "图形着色器",
    // LANG_GRAPHICS_SMOOTH
    "平滑图像",
    // LANG_OVERLAY_SELECT
    "遮罩图选择",
    // LANG_OVERLAY_MODE
    "遮罩图显示模式",
    // LANG_SHOW_FPS
    "显示帧数",

    // LANG_DISPLAY_SIZE_1X
    "1倍大小",
    // LANG_DISPLAY_SIZE_2X
    "2倍大小",
    // LANG_DISPLAY_SIZE_3X
    "3倍大小",
    // LANG_DISPLAY_SIZE_FULL
    "铺满屏幕",

    // LANG_ASPECT_RATIO_BY_GAME_RESOLUTION
    "由游戏分辨率",
    // LANG_ASPECT_RATIO_BY_DEV_SCREEN
    "由设备屏幕",
    // LANG_ASPECT_RATIO_8_7
    "8:7",
    // LANG_ASPECT_RATIO_4_3
    "4:3",
    // LANG_ASPECT_RATIO_3_2
    "3:2",
    // LANG_ASPECT_RATIO_16_9
    "16:9",

    // LANG_DISPLAY_ROTATE_CW_90
    "旋转90度(顺时针)",
    // LANG_DISPLAY_ROTATE_CW_180
    "旋转180度(顺时针)",
    // LANG_DISPLAY_ROTATE_CW_270
    "旋转270度(顺时针)",

    // LANG_SHADER_LCD3X
    "LCD3x",
    // LANG_SHADER_SHARP_BILINEAR_SIMPLE
    "锐利双线性",
    // LANG_SHADER_SHARP_BILINEAR
    "锐利双线性+扫描线",
    // LANG_SHADER_ADVANCED_AA
    "高级AA",

    // LANG_OVERLAY_MODE_OVERLAY
    "覆层模式",
    // LANG_OVERLAY_MODE_BACKGROUND
    "背景模式",

    /**************  Menu control  *****************/
    // LANG_CONTROLLER_PORT
    "控制器端口",
    // LANG_FRONT_TOUCH_TO_BUTTON
    "前触摸映射按键",
    // LANG_BACK_TOUCH_TO_BUTTON
    "背触摸映射按键",
    // LANG_TURBO_DELAY
    "连发间隔 (帧)",

    /**************  Menu misc  *****************/
    // LANG_AUTO_SAVE_LOAD_STATE
    "自动存读档",
    // LANG_ENABLE_REWIND
    "启用回溯",
    // LANG_REWIND_MAX_COUNT
    "最大回溯次数",
    // LANG_REWIND_INTERVAL_TIME
    "回溯间隔时间 (秒)",
    // LANG_SAVE_SCREENSHOT
    "保存截图",
    // LANG_SAVE_PREVIEW
    "保存截图为预览图",

    // LANG_SAVE_SCREENSHOT_OK,
    "保存截图完成！",
    // LANG_SAVE_SCREENSHOT_FAILED,
    "保存截图失败！",
    // LANG_SAVE_PREVIEW_OK,
    "保存截图为预览图完成！",
    // LANG_SAVE_PREVIEW_FAILED,
    "保存截图为预览图失败！",

    /**************  Menu hotkey  *****************/
    // LANG_HOTKEY_SAVESTATE
    "[快捷键] 保存存档",
    // LANG_HOTKEY_LOADSTATE
    "[快捷键] 读取存档",
    // LANG_HOTKEY_GAME_SPEED_UP
    "[快捷键] 加速游戏",
    // LANG_HOTKEY_GAME_SPEED_DOWN
    "[快捷键] 减速游戏",
    // LANG_HOTKEY_GAME_REWIND
    "[快捷键] 回溯游戏",
    // LANG_HOTKEY_CONTROLLER_PORT_UP
    "[快捷键] 控制器端口+",
    // LANG_HOTKEY_CONTROLLER_PORT_DOWN
    "[快捷键] 控制器端口-",
    // LANG_HOTKEY_EXIT_GAME
    "[快捷键] 退出游戏",

    /**************  Menu app  *****************/
    // LANG_PREVIEW_PATH
    "预览图路径",
    // LANG_PREVIEW_STYLE
    "预览图样式",
    // LANG_APP_LOG
    "程序日志",
    // LANG_CORE_LOG
    "核心日志",
    // LANG_SHOW_LOG
    "显示日志 (加载游戏时)",
    // LANG_LANGUAGE
    "语言",

    // LANG_PREVIEW_PATH_FROM_AUTO_STATE
    "从自动即时存档",

    // LANG_PREVIEW_STYLE_PRESERVE_FULL
    "等比铺满",
    // LANG_PREVIEW_STYLE_STRETCH_FULL
    "拉伸铺满",
    // LANG_PREVIEW_STYLE_CROP_FULL,
    "裁剪铺满",

    /**************  Menu state  *****************/
    // LANG_STATE_EXISTENT_STATE
    "已存档",
    // LANG_STATE_EMPTY_STATE
    "未存档",

    // LANG_STATE_LOAD_STATE
    "读取",
    // LANG_STATE_SAVE_STATE
    "保存",
    // LANG_STATE_DELETE_STATE
    "删除",
};

/* 繁體中文 */
char *lang_cht[LANGUAGE_CONTAINER_SIZE] = {
    // LANG_NULL
    NULL,

    /**************  General  *****************/
    // LANG_YES
    "是",
    // LANG_NO
    "否",
    // LANG_CONFIRM
    "確定",
    // LANG_CANCEL
    "取消",
    // LANG_BACK
    "返回",
    // LANG_EXIT
    "退出",

    /**************  Dialog  *****************/
    // LANG_TIP
    "提示",
    // LANG_MENU
    "菜單",
    // LANG_COLSE
    "關閉",

    /**************  Safe mode  *****************/
    // LANG_MESSAGE_WARN_SAFE_MODE
    "當前處於安全模式，請先在HENkaku設置裏開啟啟用不安全自製軟件，"
    "然後才能正常使用本程序。",

    /**************  Button string  *****************/
    // LANG_BUTTON_LEFT
    "←",
    // LANG_BUTTON_UP
    "↑",
    // LANG_BUTTON_RIGHT
    "→",
    // LANG_BUTTON_DOWN
    "↓",
    // LANG_BUTTON_A
    "A",
    // LANG_BUTTON_B
    "B",
    // LANG_BUTTON_X
    "X",
    // LANG_BUTTON_Y
    "Y",
    // LANG_BUTTON_L
    "L",
    // LANG_BUTTON_R
    "R",
    // LANG_BUTTON_L1
    "L1",
    // LANG_BUTTON_R1
    "R1",
    // LANG_BUTTON_L2
    "L2",
    // LANG_BUTTON_R2
    "R2",
    // LANG_BUTTON_L3
    "L3",
    // LANG_BUTTON_R3
    "R3",
    // LANG_BUTTON_SELECT
    "Select",
    // LANG_BUTTON_START
    "Start",
    // LANG_BUTTON_HOME
    "Home",
    // LANG_BUTTON_LEFT_ANALOG_LEFT
    "左搖桿←",
    // LANG_BUTTON_LEFT_ANALOG_UP
    "左搖桿↑",
    // LANG_BUTTON_LEFT_ANALOG_RIGHT
    "左搖桿→",
    // LANG_BUTTON_LEFT_ANALOG_DOWN
    "左搖桿↓",
    // LANG_BUTTON_RIGHT_ANALOG_LEFT
    "右搖桿←",
    // LANG_BUTTON_RIGHT_ANALOG_UP
    "右搖桿↑",
    // LANG_BUTTON_RIGHT_ANALOG_RIGHT
    "右搖桿→",
    // LANG_BUTTON_RIGHT_ANALOG_DOWN
    "右搖桿↓",

    // LANG_BUTTON_ENTER
    NULL,
    // LANG_BUTTON_CANCEL
    NULL,

    /**************  Button string 2  *****************/
    // LANG_BUTTON_CROSS
    "╳",
    // LANG_BUTTON_CIRCLE
    "○",
    // LANG_BUTTON_SQUARE
    "□",
    // LANG_BUTTON_TRIANGLE
    "△",

    // LANG_BUTTON_C
    "C",
    // LANG_BUTTON_D
    "D",
    // LANG_BUTTON_E
    "E",
    // LANG_BUTTON_F
    "F",
    // LANG_BUTTON_G
    "G",
    // LANG_BUTTON_H
    "H",
    // LANG_BUTTON_Z
    "Z",

    // LANG_BUTTON_X1
    "X1",
    // LANG_BUTTON_X2
    "X2",
    // LANG_BUTTON_X3
    "X3",
    // LANG_BUTTON_X4
    "X4",
    // LANG_BUTTON_Y1
    "Y1",
    // LANG_BUTTON_Y2
    "Y2",
    // LANG_BUTTON_Y3
    "Y3",
    // LANG_BUTTON_Y4
    "Y4",

    // LANG_SWICTH_MODE
    "模式切換",
    // LANG_COIN
    "投幣",
    // LANG_SELECT
    "選擇",
    // LANG_START
    "開始",
    // LANG_TURBO
    "連發",
    // LANG_FDS_DISK_SIDE_CHANGE
    "[FDS] 更換磁盤面",
    // LANG_FDS_INSERT_EJECT_DISK
    "[FDS] 插入/彈出磁盤",

    /**************  Browser  *****************/
    // LANG_APP_TITLE
    APP_NAME " v" APP_VER,

    // LANG_PARENT_DIR
    "上層目錄",
    // LANG_OPEN_DIR
    "打開目錄",
    // LANG_START_GAME
    "啟動遊戲",
    // LANG_OPTION_MENU
    "選項菜單",
    // LANG_SETTING_MENU
    "設置菜單",
    // LANG_ABOUT
    "關於",
    // LANG_CHANGE_DIR
    "跳轉目錄",

    // LANG_OPTION_MENU_START_GAME
    "啟動遊戲",
    // LANG_OPTION_MENU_DELETE_GAME
    "刪除遊戲",
    // LANG_OPTION_MENU_DELETE_AUTO_SAVESTATE
    "刪除自動即時存檔",
    // LANG_OPTION_MENU_DELETE_AUTO_SAVEFILE
    "刪除自動模擬存檔",
    // LANG_OPTION_MENU_DELETE_CACHE_FILES
    "刪除緩存文件",

    // LANG_MESSAGE_ASK_DELETE_GAME
    "確認要刪除這個遊戲？",
    // LANG_MESSAGE_ASK_DELETE_AUTO_STATE
    "確認要刪除這個遊戲的自動即時存檔？",
    // LANG_MESSAGE_ASK_DELETE_AUTO_SAVEFILE
    "確認要刪除這個遊戲的自動模擬存檔？",
    // LANG_MESSAGE_ASK_DELETE_CACHE_FILES
    "確認要刪除這個遊戲的緩存文件？",
    // LANG_MESSAGE_START_GAME_FAILED
    "啟動遊戲失敗！",
    // LANG_MESSAGE_WAIT_EXITING
    "正在退出，請稍候......",

    /**************  About  *****************/
    // LANG_ABOUT_TITLE
    "關於",

    /**************  Menu tab  *****************/
    // LANG_MENU_MAIN
    "主菜單",
    // LANG_MENU_STATE
    "即時存檔",
    // LANG_MENU_GRAPHICS
    "圖形",
    // LANG_MENU_CONTROL
    "控製",
    // LANG_MENU_HOTKEY
    "快捷鍵",
    // LANG_MENU_CORE
    "核心",
    // LANG_MENU_CHEAT
    "金手指",
    // LANG_MENU_MISC
    "雜項",
    // LANG_MENU_APP
    "程序",

    /**************  Menu general  *****************/
    // LANG_DISABLE
    "禁用",
    // LANG_DEFAULT
    "默認",
    // LANG_AUTO
    "自動",
    // LANG_NONE
    "無",
    // LANG_RESET_CONFIGS
    "恢復默認設置",

    /**************  Menu main  *****************/
    // LANG_RESUME_GAME
    "繼續遊戲",
    // LANG_RESET_GAME
    "重置遊戲",
    // LANG_EXIT_GAME
    "退出遊戲",
    // LANG_DISK_CONTROL
    "光盤控製",
    // LANG_EXIT_TO_ARCH
    "返回前端",
    // LANG_EXIT_APP
    "退出程序",

    // LANG_SWITCH_DISK
    "更換光盤",
    // LANG_DISK
    "光盤",
    // LANG_CURRENT
    "當前",

    /**************  Menu graphics  *****************/
    // LANG_DISPLAY_SIZE
    "畫面尺寸",
    // LANG_ASPECT_RATIO
    "畫面比例",
    // LANG_DISPLAY_ROTATE
    "畫面旋轉",
    // LANG_GRAPHICS_SHADER
    "圖形著色器",
    // LANG_GRAPHICS_SMOOTH
    "平滑圖像",
    // LANG_OVERLAY_SELECT
    "遮罩圖選擇",
    // LANG_OVERLAY_MODE
    "遮罩圖顯示模式",
    // LANG_SHOW_FPS
    "顯示幀數",

    // LANG_DISPLAY_SIZE_1X
    "1倍大小",
    // LANG_DISPLAY_SIZE_2X
    "2倍大小",
    // LANG_DISPLAY_SIZE_3X
    "3倍大小",
    // LANG_DISPLAY_SIZE_FULL
    "鋪滿屏幕",

    // LANG_ASPECT_RATIO_BY_GAME_RESOLUTION
    "由遊戲分辨率",
    // LANG_ASPECT_RATIO_BY_DEV_SCREEN
    "由設備屏幕",
    // LANG_ASPECT_RATIO_8_7
    "8:7",
    // LANG_ASPECT_RATIO_4_3
    "4:3",
    // LANG_ASPECT_RATIO_3_2
    "3:2",
    // LANG_ASPECT_RATIO_16_9
    "16:9",

    // LANG_DISPLAY_ROTATE_CW_90
    "旋轉90度(順時針)",
    // LANG_DISPLAY_ROTATE_CW_180
    "旋轉180度(順時針)",
    // LANG_DISPLAY_ROTATE_CW_270
    "旋轉270度(順時針)",

    // LANG_SHADER_LCD3X
    "LCD3x",
    // LANG_SHADER_SHARP_BILINEAR_SIMPLE
    "銳利雙線性",
    // LANG_SHADER_SHARP_BILINEAR
    "銳利雙線性+掃描線",
    // LANG_SHADER_ADVANCED_AA
    "高級AA",

    // LANG_OVERLAY_MODE_OVERLAY
    "覆層模式",
    // LANG_OVERLAY_MODE_BACKGROUND
    "背景模式",

    /**************  Menu control  *****************/
    // LANG_CONTROLLER_PORT
    "控製器端口",
    // LANG_FRONT_TOUCH_TO_BUTTON
    "前觸摸映射按鍵",
    // LANG_BACK_TOUCH_TO_BUTTON
    "背觸摸映射按鍵",
    // LANG_TURBO_DELAY
    "連發間隔 (幀)",

    /**************  Menu misc  *****************/
    // LANG_AUTO_SAVE_LOAD_STATE
    "自動存讀檔",
    // LANG_ENABLE_REWIND
    "開啟回溯",
    // LANG_REWIND_MAX_COUNT
    "最大回溯次數",
    // LANG_REWIND_INTERVAL_TIME
    "回溯間隔時間 (秒)",
    // LANG_SAVE_SCREENSHOT
    "保存截圖",
    // LANG_SAVE_PREVIEW
    "保存截圖為預覽圖",

    // LANG_SAVE_SCREENSHOT_OK,
    "保存截圖完成！",
    // LANG_SAVE_SCREENSHOT_FAILED,
    "保存截圖失敗！",
    // LANG_SAVE_PREVIEW_OK,
    "保存截圖為預覽圖完成！",
    // LANG_SAVE_PREVIEW_FAILED,
    "保存截圖為預覽圖失敗！",

    /**************  Menu hotkey  *****************/
    // LANG_HOTKEY_SAVESTATE
    "[快捷鍵] 保存存檔",
    // LANG_HOTKEY_LOADSTATE
    "[快捷鍵] 讀取存檔",
    // LANG_HOTKEY_GAME_SPEED_UP
    "[快捷鍵] 加速遊戲",
    // LANG_HOTKEY_GAME_SPEED_DOWN
    "[快捷鍵] 減速遊戲",
    // LANG_HOTKEY_GAME_REWIND
    "[快捷鍵] 回溯遊戲",
    // LANG_HOTKEY_CONTROLLER_PORT_UP
    "[快捷鍵] 控製器端口+",
    // LANG_HOTKEY_CONTROLLER_PORT_DOWN
    "[快捷鍵] 控製器端口-",
    // LANG_HOTKEY_EXIT_GAME
    "[快捷鍵] 退出遊戲",

    /**************  Menu app  *****************/
    // LANG_PREVIEW_PATH
    "預覽圖路徑",
    // LANG_PREVIEW_STYLE
    "預覽圖樣式",
    // LANG_APP_LOG
    "程序日誌",
    // LANG_CORE_LOG
    "核心日誌",
    // LANG_SHOW_LOG
    "顯示日誌 (加載遊戲時)",
    // LANG_LANGUAGE
    "語言",

    // LANG_PREVIEW_PATH_FROM_AUTO_STATE
    "從自動即時存檔",

    // LANG_PREVIEW_STYLE_PRESERVE_FULL
    "等比鋪滿",
    // LANG_PREVIEW_STYLE_STRETCH_FULL
    "拉伸鋪滿",
    // LANG_PREVIEW_STYLE_CROP_FULL,
    "裁剪鋪滿",

    /**************  Menu state  *****************/
    // LANG_STATE_EXISTENT_STATE
    "已存檔",
    // LANG_STATE_EMPTY_STATE
    "未存檔",

    // LANG_STATE_LOAD_STATE
    "讀取",
    // LANG_STATE_SAVE_STATE
    "保存",
    // LANG_STATE_DELETE_STATE
    "刪除",
};

#endif