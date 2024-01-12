#ifndef __M_LANG_INTL_H__
#define __M_LANG_INTL_H__

#include "lang.h"

/* English */
char *lang_us[LANGUAGE_CONTAINER_SIZE] = {
    // Null
    NULL,

    /**************  General  *****************/
    // YES
    "yes",
    // NO
    "no",
    // CONFIRM
    "confirm",
    // CANCEL
    "cancel",
    // BACK
    "back",
    // EXIT
    "exit",

    /**************  Alert dialog  *****************/
    // TITLE_TIP
    "Tip",
    // TITLE_MENU
    "Menu",
    // COLSE
    "close",

    /**************  Safe mode  *****************/
    // MESSAGE_SAFE_MODE
    "Currently in safe mode, please enable unsafe homemade software in HENkaku settings first, "
    "only then can this program be used normally.",

    /**************  Button string  *****************/
    // BUTTON_ENTER
    NULL,
    // BUTTON_CANCEL
    NULL,
    // BUTTON_LEFT
    "←",
    // BUTTON_UP
    "↑",
    // BUTTON_RIGHT
    "→",
    // BUTTON_DOWN
    "↓",
    // BUTTON_CROSS
    "×",
    // BUTTON_CIRCLE
    "○",
    // BUTTON_SQUARE
    "□",
    // BUTTON_TRIANGLE
    "△",
    // BUTTON_L
    "L",
    // BUTTON_R
    "R",
    // BUTTON_L2
    "L2",
    // BUTTON_R2
    "R2",
    // BUTTON_L3
    "L3",
    // BUTTON_R3
    "R3",
    // BUTTON_SELECT
    "Select",
    // BUTTON_START
    "Start",
    // BUTTON_PSBUTTON
    "Home",
    // BUTTON_LEFT_ANALOG
    "Left analog",
    // BUTTON_LEFT_ANALOG_LEFT
    "Left analog←",
    // BUTTON_LEFT_ANALOG_UP
    "Left analog↑",
    // BUTTON_LEFT_ANALOG_RIGHT
    "Left analog→",
    // BUTTON_LEFT_ANALOG_DOWN
    "Left analog↓",
    // BUTTON_RIGHT_ANALOG
    "Right analog",
    // BUTTON_RIGHT_ANALOG_LEFT
    "Right analog←",
    // BUTTON_RIGHT_ANALOG_UP
    "Right analog↑",
    // BUTTON_RIGHT_ANALOG_RIGHT
    "Right analog→",
    // BUTTON_RIGHT_ANALOG_DOWN
    "Right analog↓",

    /**************  Button string 2  *****************/
    // BUTTON_A
    "A",
    // BUTTON_B
    "B",
    // BUTTON_C
    "C",
    // BUTTON_D
    "D",
    // BUTTON_E
    "E",
    // BUTTON_F
    "F",
    // BUTTON_G
    "G",
    // BUTTON_H
    "H",
    // BUTTON_X
    "X",
    // BUTTON_Y
    "Y",
    // BUTTON_Z
    "Z",

    // BUTTON_X1
    "X1",
    // BUTTON_X2
    "X2",
    // BUTTON_X3
    "X3",
    // BUTTON_X4
    "X4",
    // BUTTON_Y1
    "Y1",
    // BUTTON_Y2
    "Y2",
    // BUTTON_Y3
    "Y3",
    // BUTTON_Y4
    "Y4",

    // LABEL_SWICTH_MODE
    "Swicth mode",
    // LABEL_COIN
    "Coin",
    // LABEL_SELECT
    "Select",
    // LABEL_START
    "Start",
    // LABEL_TURBO
    "Turbo",
    // LABEL_FDS_DISK_SIDE_CHANGE
    "(FDS) Disk side change",
    // LABEL_FDS_INSERT_EJECT_DISK
    "(FDS) Insert/Eject disk",

    /**************  Browser  *****************/
    // APP_TITLE
    APP_NAME_STR " v" APP_VER_STR,

    // PARENT_DIRECTORY
    "parent dir",
    // OPEN_DIR
    "open dir",
    // START_GAME
    "start game",
    // OPTION_MENU
    "option menu",
    // SETTING_MENU
    "setting menu",
    // ABOUT
    "about",
    // CHANGE_DIR
    "change dir",

    // LABEL_START_GAME
    "Start game",
    // LABEL_DELETE_GAME
    "Delete game",
    // LABEL_DELETE_AUTO_STATE
    "Delete auto state",
    // LABEL_DELETE_SAVEFILE
    "Delete savefile",

    // MESSAGE_ASK_DELETE_GAME
    "Are you sure you want to delete this game?",
    // MESSAGE_ASK_DELETE_AUTO_STATE
    "Are you sure you want to delete the auto state of this game?",
    // MESSAGE_ASK_DELETE_SAVEFILE
    "Are you sure you want to delete the save file of this game?",
    // MESSAGE_START_GAME_FAILED
    "Failed to start this game!",

    /**************  About  *****************/
    // ABOUT_TITLE
    "About",

    /**************  Setting table  *****************/
    // TAB_MAIN
    "Main",
    // TAB_STATE
    "State",
    // TAB_GRAPHICS
    "Graphics",
    // TAB_CONTROL
    "Control",
    // TAB_HOTKEY
    "HotKey",
    // TAB_CORE
    "Core",
    // TAB_CHEAT
    "Cheat",
    // TAB_MISC
    "Misc",
    // TAB_APP
    "App",

    /**************  Menu general  *****************/
    // DISABLE
    "disable",
    // DEFAULT
    "default",
    // AUTO
    "auto",
    // NONE
    "none",
    // LABEL_RESET_CONFIGS
    "Reset configs",

    /**************  Menu main  *****************/
    // LABEL_RESUME_GAME
    "Resume game",
    // LABEL_RESET_GAME
    "Reset game",
    // LABEL_EXIT_GAME
    "Exit game",
    // LABEL_DISK_CONTROL
    "Disk control",
    // LABEL_EXIT_TO_ARCH
    "Eixt to arch",
    // LABEL_EXIT_APP
    "Exit app",

    // TITLE_SWITCH_DISK
    "Switch disk",
    // LABEL_DISK
    "Disk",
    // CURRENT
    "current",

    /**************  Menu graphics  *****************/
    // LABEL_DISPLAY_SIZE
    "Display size",
    // LABEL_ASPECT_RATIO
    "Aspect ratio",
    // LABEL_DISPLAY_ROTATE
    "Display rotate",
    // LABEL_GRAPHICS_SHADER
    "Graphics shader",
    // LABEL_GRAPHICS_SMOOTH
    "Graphics smooth",
    // LABEL_OVERLAY_SELECT
    "Overlay select",
    // LABEL_OVERLAY_MODE
    "Overlay mode",
    // LABEL_SHOW_FPS
    "Show FPS",

    // DISPLAY_SIZE_1X
    "1X",
    // DISPLAY_SIZE_2X
    "2X",
    // DISPLAY_SIZE_3X
    "3X",
    // DISPLAY_SIZE_FULL
    "full",

    // ASPECT_RATIO_BY_GAME_RESOLUTION
    "by game resolution",
    // ASPECT_RATIO_BY_DEV_SCREEN
    "by device screen",
    // ASPECT_RATIO_8_7
    "8:7",
    // ASPECT_RATIO_4_3
    "4:3",
    // ASPECT_RATIO_3_2
    "3:2",
    // ASPECT_RATIO_16_9
    "16:9",

    // DISPLAY_ROTATE_CW_90
    "rotate 90°CW",
    // DISPLAY_ROTATE_CW_180
    "rotate 180°CW",
    // DISPLAY_ROTATE_CW_270
    "rotate 270°CW",

    // SHADER_LCD3X
    "LCD3x",
    // SHADER_SHARP_BILINEAR_SIMPLE
    "sharp bilinear simple",
    // SHADER_SHARP_BILINEAR
    "sharp bilinear",
    // SHADER_ADVANCED_AA
    "advanced AA",

    // OVERLAY_MODE_OVERLAY
    "overlay",
    // OVERLAY_MODE_BACKGROUND
    "background",

    /**************  Menu control  *****************/
    // LABEL_CTRL_PLAYER
    "Control player",
    // LABEL_FRONT_TOUCH_TO_BUTTON
    "Front touch to button",
    // LABEL_BACK_TOUCH_TO_BUTTON
    "Back touch to button",
    // LABEL_TURBO_DELAY
    "Turbo delay (frame)",

    /**************  Menu misc  *****************/
    // LABEL_AUTO_SAVE_LOAD_STATE
    "Auto save/load state",
    // LABEL_SAVE_SCREENSHOT
    "Save screenshot",
    // LABEL_SAVE_PREVIEW
    "Save screenshot for preview",

    /**************  Menu hot key  *****************/
    // LABEL_HK_SAVESTATE
    "Save state (hot key)",
    // LABEL_HK_LOADSTATE
    "Load state (hot key)",
    // LABEL_HK_SPEED_UP
    "Speed up (hot key)",
    // LABEL_HK_SPEED_DOWN
    "Speed down (hot key)",
    // LABEL_HK_PLAYER_UP
    "Control player+ (hot key)",
    // LABEL_HK_PLAYER_DOWN
    "Control player- (hot key)",
    // LABEL_HK_EXIT_GAME
    "Exit game (hot key)",

    /**************  Menu app  *****************/
    // LABEL_PREVIEW_PATH
    "Preview path",
    // LABEL_PREVIEW_STYLE
    "Preview style",
    // LABEL_APP_LOG
    "App log",
    // LABEL_CORE_LOG
    "Core log",
    // LABEL_SHOW_LOG
    "Show log (loading)",
    // LABEL_LANGUAGE
    "Language",

    // FROM_AUTO_STATE
    "from auto state",

    // FULL_PRESERVE
    "preserve full",
    // FULL_STRETCH
    "stretch full",
    // FULL_CROP,
    "crop full",

    /**************  Menu state  *****************/
    // LABEL_EXISTENT_STATE
    "State",
    // LABEL_NON_EXISTENT_STATE
    "None",

    // LOAD
    "load",
    // SAVE
    "save",
    // DELETE
    "delete",
};

/* 简体中文 */
char *lang_chs[LANGUAGE_CONTAINER_SIZE] = {
    // Null
    NULL,

    /**************  General  *****************/
    // YES
    "是",
    // NO
    "否",
    // CONFIRM
    "确定",
    // CANCEL
    "取消",
    // BACK
    "返回",
    // EXIT
    "退出",

    /**************  Dialog  *****************/
    // TITLE_TIP
    "提示",
    // TITLE_MENU
    "菜单",
    // COLSE
    "关闭",

    /**************  Safe mode  *****************/
    // MESSAGE_SAFE_MODE
    "当前处于安全模式，请先在HENkaku设置里开启启用不安全自制软件，"
    "然后才能正常使用本程序。",

    /**************  Button string  *****************/
    // BUTTON_ENTER
    NULL,
    // BUTTON_CANCEL
    NULL,
    // BUTTON_LEFT
    "←",
    // BUTTON_UP
    "↑",
    // BUTTON_RIGHT
    "→",
    // BUTTON_DOWN
    "↓",
    // BUTTON_CROSS
    "×",
    // BUTTON_CIRCLE
    "○",
    // BUTTON_SQUARE
    "□",
    // BUTTON_TRIANGLE
    "△",
    // BUTTON_L
    "L",
    // BUTTON_R
    "R",
    // BUTTON_L2
    "L2",
    // BUTTON_R2
    "R2",
    // BUTTON_L3
    "L3",
    // BUTTON_R3
    "R3",
    // BUTTON_SELECT
    "Select",
    // BUTTON_START
    "Start",
    // BUTTON_PSBUTTON
    "Home",
    // BUTTON_LEFT_ANALOG
    "左摇杆",
    // BUTTON_LEFT_ANALOG_LEFT
    "左摇杆←",
    // BUTTON_LEFT_ANALOG_UP
    "左摇杆↑",
    // BUTTON_LEFT_ANALOG_RIGHT
    "左摇杆→",
    // BUTTON_LEFT_ANALOG_DOWN
    "左摇杆↓",
    // BUTTON_RIGHT_ANALOG
    "右摇杆",
    // BUTTON_RIGHT_ANALOG_LEFT
    "右摇杆←",
    // BUTTON_RIGHT_ANALOG_UP
    "右摇杆↑",
    // BUTTON_RIGHT_ANALOG_RIGHT
    "右摇杆→",
    // BUTTON_RIGHT_ANALOG_DOWN
    "右摇杆↓",

    /**************  Button string 2  *****************/
    // BUTTON_A
    "A",
    // BUTTON_B
    "B",
    // BUTTON_C
    "C",
    // BUTTON_D
    "D",
    // BUTTON_E
    "E",
    // BUTTON_F
    "F",
    // BUTTON_G
    "G",
    // BUTTON_H
    "H",
    // BUTTON_X
    "X",
    // BUTTON_Y
    "Y",
    // BUTTON_Z
    "Z",

    // BUTTON_X1
    "X1",
    // BUTTON_X2
    "X2",
    // BUTTON_X3
    "X3",
    // BUTTON_X4
    "X4",
    // BUTTON_Y1
    "Y1",
    // BUTTON_Y2
    "Y2",
    // BUTTON_Y3
    "Y3",
    // BUTTON_Y4
    "Y4",

    // LABEL_SWICTH_MODE
    "模式切换",
    // LABEL_COIN
    "投币",
    // LABEL_SELECT
    "选择",
    // LABEL_START
    "开始",
    // LABEL_TURBO
    "连发",
    // LABEL_FDS_DISK_SIDE_CHANGE
    "(FDS) 更换磁盘面",
    // LABEL_FDS_INSERT_EJECT_DISK
    "(FDS) 插入/弹出磁盘",

    /**************  Browser  *****************/
    // APP_TITLE
    APP_NAME_STR " v" APP_VER_STR,

    // PARENT_DIRECTORY
    "上层目录",
    // OPEN_DIR
    "打开目录",
    // START_GAME
    "启动游戏",
    // OPTION_MENU
    "选项菜单",
    // SETTING_MENU
    "设置菜单",
    // ABOUT
    "关于",
    // CHANGE_DIR
    "跳转目录",

    // LABEL_START_GAME
    "启动游戏",
    // LABEL_DELETE_GAME
    "删除游戏",
    // LABEL_DELETE_AUTO_STATE
    "删除自动存档",
    // LABEL_DELETE_SAVEFILE
    "删除模拟存档",

    // MESSAGE_ASK_DELETE_GAME
    "确认要删除这个游戏？",
    // MESSAGE_ASK_DELETE_AUTO_STATE
    "确认要删除这个游戏的自动存档？",
    // MESSAGE_ASK_DELETE_SAVEFILE
    "确认要删除这个游戏的模拟存档？",
    // MESSAGE_START_GAME_FAILED
    "启动游戏失败！",

    /**************  About  *****************/
    // ABOUT_TITLE
    "关于",

    /**************  Setting tab  *****************/
    // TAB_MAIN
    "主菜单",
    // TAB_STATE
    "即时存档",
    // TAB_GRAPHICS
    "图形",
    // TAB_CONTROL
    "控制",
    // TAB_HOTKEY
    "热键",
    // TAB_CORE
    "核心",
    // TAB_CHEAT
    "金手指",
    // TAB_MISC
    "杂项",
    // TAB_APP
    "程序",

    /**************  Menu general  *****************/
    // DISABLE
    "禁用",
    // DEFAULT
    "默认",
    // AUTO
    "自动",
    // NONE
    "无",
    // LABEL_RESET_CONFIGS
    "恢复默认设置",

    /**************  Menu main  *****************/
    // LABEL_RESUME_GAME
    "继续游戏",
    // LABEL_RESET_GAME
    "重置游戏",
    // LABEL_EXIT_GAME
    "退出游戏",
    // LABEL_DISK_CONTROL
    "光盘控制",
    // LABEL_EXIT_TO_ARCH
    "返回前端",
    // LABEL_EXIT_APP
    "退出程序",

    // TITLE_SWITCH_DISK
    "更换光盘",
    // LABEL_DISK
    "光盘",
    // CURRENT
    "当前",

    /**************  Menu graphics  *****************/
    // LABEL_DISPLAY_SIZE
    "画面尺寸",
    // LABEL_ASPECT_RATIO
    "画面比例",
    // LABEL_DISPLAY_ROTATE
    "画面旋转",
    // LABEL_GRAPHICS_SHADER
    "图像滤境",
    // LABEL_GRAPHICS_SMOOTH
    "平滑图像",
    // LABEL_OVERLAY_SELECT
    "遮罩图选择",
    // LABEL_OVERLAY_MODE
    "遮罩图显示模式",
    // LABEL_SHOW_FPS
    "显示帧数",

    // DISPLAY_SIZE_1X
    "1倍大小",
    // DISPLAY_SIZE_2X
    "2倍大小",
    // DISPLAY_SIZE_3X
    "3倍大小",
    // DISPLAY_SIZE_FULL
    "铺满屏幕",

    // ASPECT_RATIO_BY_GAME_RESOLUTION
    "由游戏分辨率",
    // ASPECT_RATIO_BY_DEV_SCREEN
    "由设备屏幕",
    // ASPECT_RATIO_8_7
    "8:7",
    // ASPECT_RATIO_4_3
    "4:3",
    // ASPECT_RATIO_3_2
    "3:2",
    // ASPECT_RATIO_16_9
    "16:9",

    // DISPLAY_ROTATE_CW_90
    "旋转90度(顺时针)",
    // DISPLAY_ROTATE_CW_180
    "旋转180度(顺时针)",
    // DISPLAY_ROTATE_CW_270
    "旋转270度(顺时针)",

    // SHADER_LCD3X
    "LCD3x",
    // SHADER_SHARP_BILINEAR_SIMPLE
    "锐利双线性",
    // SHADER_SHARP_BILINEAR
    "锐利双线性+扫描线",
    // SHADER_ADVANCED_AA
    "高级AA",

    // OVERLAY_MODE_OVERLAY
    "覆层模式",
    // OVERLAY_MODE_BACKGROUND
    "背景模式",

    /**************  Menu control  *****************/
    // LABEL_CTRL_PLAYER
    "玩家控制",
    // LABEL_FRONT_TOUCH_TO_BUTTON
    "前触摸映射按键",
    // LABEL_BACK_TOUCH_TO_BUTTON
    "背触摸映射按键",
    // LABEL_TURBO_DELAY
    "连发间隔 (帧)",

    /**************  Menu misc  *****************/
    // LABEL_AUTO_SAVE_LOAD_STATE
    "自动存读档",
    // LABEL_SAVE_SCREENSHOT
    "保存截图",
    // LABEL_SAVE_PREVIEW
    "保存截图为预览图",


    /**************  Menu hot key  *****************/
    // LABEL_HK_SAVESTATE
    "保存存档 (快捷键)",
    // LABEL_HK_LOADSTATE
    "读取存档 (快捷键)",
    // LABEL_HK_SPEED_UP
    "加速游戏 (快捷键)",
    // LABEL_HK_SPEED_DOWN
    "减速游戏 (快捷键)",
    // LABEL_HK_PLAYER_UP
    "切换玩家+ (快捷键)",
    // LABEL_HK_PLAYER_DOWN
    "切换玩家- (快捷键)",
    // LABEL_HK_EXIT_GAME
    "退出游戏 (快捷键)",

    /**************  Menu app  *****************/
    // LABEL_PREVIEW_PATH
    "预览图路径",
    // LABEL_PREVIEW_STYLE
    "预览图样式",
    // LABEL_APP_LOG
    "程序日志",
    // LABEL_CORE_LOG
    "核心日志",
    // LABEL_SHOW_LOG
    "显示日志 (加载游戏时)",
    // LABEL_LANGUAGE
    "语言",

    // FROM_AUTO_STATE
    "从自动存档",

    // FULL_PRESERVE
    "等比铺满",
    // FULL_STRETCH
    "拉伸铺满",
    // FULL_CROP,
    "裁剪铺满",

    /**************  Menu state  *****************/
    // LABEL_EXISTENT_STATE
    "已存档",
    // LABEL_NON_EXISTENT_STATE
    "未存档",

    // LOAD
    "读取",
    // SAVE
    "保存",
    // DELETE
    "删除",
};

/* 繁體中文 */
char *lang_cht[LANGUAGE_CONTAINER_SIZE] = {
    // Null
    NULL,

    /**************  General  *****************/
    // YES
    "是",
    // NO
    "否",
    // CONFIRM
    "確定",
    // CANCEL
    "取消",
    // BACK
    "返回",
    // EXIT
    "退出",

    /**************  Dialog  *****************/
    // TITLE_TIP
    "提示",
    // TITLE_MENU
    "菜單",
    // COLSE
    "關閉",

    /**************  Safe mode  *****************/
    // MESSAGE_SAFE_MODE
    "當前處於安全模式，請先在HENkaku設置裏開啟啟用不安全自製軟件，"
    "然後才能正常使用本程序。",

    /**************  Button string  *****************/
    // BUTTON_ENTER
    NULL,
    // BUTTON_CANCEL
    NULL,
    // BUTTON_LEFT
    "←",
    // BUTTON_UP
    "↑",
    // BUTTON_RIGHT
    "→",
    // BUTTON_DOWN
    "↓",
    // BUTTON_CROSS
    "×",
    // BUTTON_CIRCLE
    "○",
    // BUTTON_SQUARE
    "□",
    // BUTTON_TRIANGLE
    "△",
    // BUTTON_L
    "L",
    // BUTTON_R
    "R",
    // BUTTON_L2
    "L2",
    // BUTTON_R2
    "R2",
    // BUTTON_L3
    "L3",
    // BUTTON_R3
    "R3",
    // BUTTON_SELECT
    "Select",
    // BUTTON_START
    "Start",
    // BUTTON_PSBUTTON
    "Home",
    // BUTTON_LEFT_ANALOG
    "左搖桿",
    // BUTTON_LEFT_ANALOG_LEFT
    "左搖桿←",
    // BUTTON_LEFT_ANALOG_UP
    "左搖桿↑",
    // BUTTON_LEFT_ANALOG_RIGHT
    "左搖桿→",
    // BUTTON_LEFT_ANALOG_DOWN
    "左搖桿↓",
    // BUTTON_RIGHT_ANALOG
    "右搖桿",
    // BUTTON_RIGHT_ANALOG_LEFT
    "右搖桿←",
    // BUTTON_RIGHT_ANALOG_UP
    "右搖桿↑",
    // BUTTON_RIGHT_ANALOG_RIGHT
    "右搖桿→",
    // BUTTON_RIGHT_ANALOG_DOWN
    "右搖桿↓",

    /**************  Button string 2  *****************/
    // BUTTON_A
    "A",
    // BUTTON_B
    "B",
    // BUTTON_C
    "C",
    // BUTTON_D
    "D",
    // BUTTON_E
    "E",
    // BUTTON_F
    "F",
    // BUTTON_G
    "G",
    // BUTTON_H
    "H",
    // BUTTON_X
    "X",
    // BUTTON_Y
    "Y",
    // BUTTON_Z
    "Z",

    // BUTTON_X1
    "X1",
    // BUTTON_X2
    "X2",
    // BUTTON_X3
    "X3",
    // BUTTON_X4
    "X4",
    // BUTTON_Y1
    "Y1",
    // BUTTON_Y2
    "Y2",
    // BUTTON_Y3
    "Y3",
    // BUTTON_Y4
    "Y4",

    // LABEL_SWICTH_MODE
    "模式切換",
    // LABEL_COIN
    "投幣",
    // LABEL_SELECT
    "選擇",
    // LABEL_START
    "開始",
    // LABEL_TURBO
    "連發",
    // LABEL_FDS_DISK_SIDE_CHANGE
    "(FDS) 更換磁盤面",
    // LABEL_FDS_INSERT_EJECT_DISK
    "(FDS) 插入/彈出磁盤",

    /**************  Browser  *****************/
    // APP_TITLE
    APP_NAME_STR " v" APP_VER_STR,

    // PARENT_DIRECTORY
    "上層目錄",
    // OPEN_DIR
    "打開目錄",
    // START_GAME
    "啟動遊戲",
    // OPTION_MENU
    "選項菜單",
    // SETTING_MENU
    "設置菜單",
    // ABOUT
    "關於",
    // CHANGE_DIR
    "跳轉目錄",

    // LABEL_START_GAME
    "啟動遊戲",
    // LABEL_DELETE_GAME
    "刪除遊戲",
    // LABEL_DELETE_AUTO_STATE
    "刪除自動存檔",
    // LABEL_DELETE_SAVEFILE
    "刪除模擬存檔",

    // MESSAGE_ASK_DELETE_GAME
    "確認要刪除這個遊戲？",
    // MESSAGE_ASK_DELETE_AUTO_STATE
    "確認要刪除這個遊戲的自動存檔？",
    // MESSAGE_ASK_DELETE_SAVEFILE
    "確認要刪除這個遊戲的模擬存檔？",
    // MESSAGE_START_GAME_FAILED
    "啟動遊戲失敗！",

    /**************  About  *****************/
    // ABOUT_TITLE
    "關於",

    /**************  Setting tab  *****************/
    // TAB_MAIN
    "主菜單",
    // TAB_STATE
    "即時存檔",
    // TAB_GRAPHICS
    "圖形",
    // TAB_CONTROL
    "控製",
    // TAB_HOTKEY
    "熱鍵",
    // TAB_CORE
    "核心",
    // TAB_CHEAT
    "金手指",
    // TAB_MISC
    "雜項",
    // TAB_APP
    "程序",

    /**************  Menu general  *****************/
    // DISABLE
    "禁用",
    // DEFAULT
    "默認",
    // AUTO
    "自動",
    // NONE
    "無",
    // LABEL_RESET_CONFIGS
    "恢復默認設置",

    /**************  Menu main  *****************/
    // LABEL_RESUME_GAME
    "繼續遊戲",
    // LABEL_RESET_GAME
    "重置遊戲",
    // LABEL_EXIT_GAME
    "退出遊戲",
    // LABEL_DISK_CONTROL
    "光盤控製",
    // LABEL_EXIT_TO_ARCH
    "返回前端",
    // LABEL_EXIT_APP
    "退出程序",

    // TITLE_SWITCH_DISK
    "更換光盤",
    // LABEL_DISK
    "光盤",
    // CURRENT
    "當前",

    /**************  Menu graphics  *****************/
    // LABEL_DISPLAY_SIZE
    "畫面尺寸",
    // LABEL_ASPECT_RATIO
    "畫面比例",
    // LABEL_DISPLAY_ROTATE
    "畫面旋轉",
    // LABEL_GRAPHICS_SHADER
    "圖像濾境",
    // LABEL_GRAPHICS_SMOOTH
    "平滑圖像",
    // LABEL_OVERLAY_SELECT
    "遮罩圖選擇",
    // LABEL_OVERLAY_MODE
    "遮罩圖顯示模式",
    // LABEL_SHOW_FPS
    "顯示幀數",

    // DISPLAY_SIZE_1X
    "1倍大小",
    // DISPLAY_SIZE_2X
    "2倍大小",
    // DISPLAY_SIZE_3X
    "3倍大小",
    // DISPLAY_SIZE_FULL
    "鋪滿屏幕",

    // ASPECT_RATIO_BY_GAME_RESOLUTION
    "由遊戲分辨率",
    // ASPECT_RATIO_BY_DEV_SCREEN
    "由設備屏幕",
    // ASPECT_RATIO_8_7
    "8:7",
    // ASPECT_RATIO_4_3
    "4:3",
    // ASPECT_RATIO_3_2
    "3:2",
    // ASPECT_RATIO_16_9
    "16:9",

    // DISPLAY_ROTATE_CW_90
    "旋轉90度(順時針)",
    // DISPLAY_ROTATE_CW_180
    "旋轉180度(順時針)",
    // DISPLAY_ROTATE_CW_270
    "旋轉270度(順時針)",

    // SHADER_LCD3X
    "LCD3x",
    // SHADER_SHARP_BILINEAR_SIMPLE
    "銳利雙線性",
    // SHADER_SHARP_BILINEAR
    "銳利雙線性+掃描線",
    // SHADER_ADVANCED_AA
    "高級AA",

    // OVERLAY_MODE_OVERLAY
    "覆層模式",
    // OVERLAY_MODE_BACKGROUND
    "背景模式",

    /**************  Menu control  *****************/
    // LABEL_CTRL_PLAYER
    "玩家控製",
    // LABEL_FRONT_TOUCH_TO_BUTTON
    "前觸摸映射按鍵",
    // LABEL_BACK_TOUCH_TO_BUTTON
    "背觸摸映射按鍵",
    // LABEL_TURBO_DELAY
    "連發間隔 (幀)",

    /**************  Menu misc  *****************/
    // LABEL_AUTO_SAVE_LOAD_STATE
    "自動存讀檔",
    // LABEL_SAVE_SCREENSHOT
    "保存截圖",
    // LABEL_SAVE_PREVIEW
    "保存截圖為預覽圖",


    /**************  Menu hot key  *****************/
    // LABEL_HK_SAVESTATE
    "保存存檔 (快捷鍵)",
    // LABEL_HK_LOADSTATE
    "讀取存檔 (快捷鍵)",
    // LABEL_HK_SPEED_UP
    "加速遊戲 (快捷鍵)",
    // LABEL_HK_SPEED_DOWN
    "減速遊戲 (快捷鍵)",
    // LABEL_HK_PLAYER_UP
    "切換玩家+ (快捷鍵)",
    // LABEL_HK_PLAYER_DOWN
    "切換玩家- (快捷鍵)",
    // LABEL_HK_EXIT_GAME
    "退出遊戲 (快捷鍵)",

    /**************  Menu app  *****************/
    // LABEL_PREVIEW_PATH
    "預覽圖路徑",
    // LABEL_PREVIEW_STYLE
    "預覽圖樣式",
    // LABEL_APP_LOG
    "程序日誌",
    // LABEL_CORE_LOG
    "核心日誌",
    // LABEL_SHOW_LOG
    "顯示日誌 (加載遊戲時)",
    // LABEL_LANGUAGE
    "語言",

    // FROM_AUTO_STATE
    "從自動存檔",

    // FULL_PRESERVE
    "等比鋪滿",
    // FULL_STRETCH
    "拉伸鋪滿",
    // FULL_CROP,
    "裁剪鋪滿",

    /**************  Menu state  *****************/
    // LABEL_EXISTENT_STATE
    "已存檔",
    // LABEL_NON_EXISTENT_STATE
    "未存檔",

    // LOAD
    "讀取",
    // SAVE
    "保存",
    // DELETE
    "刪除",
};

#endif