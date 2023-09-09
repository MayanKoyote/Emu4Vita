#ifndef __M_LANG_H__
#define __M_LANG_H__

enum LanguageContainer
{
    // Null
    LANG_NULL,

    // General
    YES,
    NO,
    CONFIRM,
    CANCEL,
    BACK,

    // Dialog
    TITLE_TIP,
    TITLE_MENU,
    COLSE,

    // Safe mode
    MESSAGE_SAFE_MODE_0,
    MESSAGE_SAFE_MODE_1,
    MESSAGE_SAFE_MODE_2,

    // Button string
    BUTTON_ENTER,
    BUTTON_CANCEL,

    BUTTON_LEFT,
    BUTTON_UP,
    BUTTON_RIGHT,
    BUTTON_DOWN,
    BUTTON_CROSS,
    BUTTON_CIRCLE,
    BUTTON_SQUARE,
    BUTTON_TRIANGLE,
    BUTTON_L,
    BUTTON_R,
    BUTTON_L2,
    BUTTON_R2,
    BUTTON_L3,
    BUTTON_R3,
    BUTTON_SELECT,
    BUTTON_START,
    BUTTON_PSBUTTON,
    BUTTON_LEFT_ANALOG,
    BUTTON_LEFT_ANALOG_LEFT,
    BUTTON_LEFT_ANALOG_UP,
    BUTTON_LEFT_ANALOG_RIGHT,
    BUTTON_LEFT_ANALOG_DOWN,
    BUTTON_RIGHT_ANALOG,
    BUTTON_RIGHT_ANALOG_LEFT,
    BUTTON_RIGHT_ANALOG_UP,
    BUTTON_RIGHT_ANALOG_RIGHT,
    BUTTON_RIGHT_ANALOG_DOWN,

    // Button string 2
    BUTTON_A,
    BUTTON_B,
    BUTTON_C,
    BUTTON_D,
    BUTTON_E,
    BUTTON_F,
    BUTTON_G,
    BUTTON_H,
    BUTTON_X,
    BUTTON_Y,
    BUTTON_Z,

    BUTTON_X1,
    BUTTON_X2,
    BUTTON_X3,
    BUTTON_X4,
    BUTTON_Y1,
    BUTTON_Y2,
    BUTTON_Y3,
    BUTTON_Y4,

    LABEL_SWICTH_MODE,
    LABEL_COIN,
    LABEL_SELECT,
    LABEL_START,
    LABEL_TURBO,

    // Browser
    APP_TITLE,

    PARENT_DIRECTORY,
    OPEN_DIR,
    START_GAME,
    OPTION_MENU,
    SETTING_MENU,
    ABOUT,
    CHANGE_DIR,

    LABEL_START_GAME,
    LABEL_DELETE_GAME,
    LABEL_DELETE_AUTO_STATE,
    LABEL_DELETE_SAVEFILE,

    MESSAGE_ASK_DELETE_GAME,
    MESSAGE_ASK_DELETE_AUTO_STATE,
    MESSAGE_ASK_DELETE_SAVEFILE,
    MESSAGE_START_GAME_FAILED,

    // About
    ABOUT_TITLE,

    // Setting tab
    TAB_MAIN,
    TAB_STATE,
    TAB_GRAPHICS,
    TAB_CONTROL,
    TAB_HOTKEY,
    TAB_CORE,
    TAB_MISC,
    TAB_APP,

    // Menu general
    DISABLE,
    DEFAULT,
    AUTO,
    NONE,
    LABEL_RESET_CONFIGS,

    // Menu main
    LABEL_RESUME_GAME,
    LABEL_RESET_GAME,
    LABEL_EXIT_GAME,
    LABEL_DISK_CONTROL,
    LABEL_EXIT_TO_ARCH,
    LABEL_EXIT_APP,

    TITLE_SWITCH_DISK,
    LABEL_DISK,
    CURRENT,

    // Menu grahics
    LABEL_DISPLAY_SIZE,
    LABEL_ASPECT_RATIO,
    LABEL_DISPLAY_ROTATE,
    LABEL_GRAHICS_SHADER,
    LABEL_GRAHICS_SMOOTH,
    LABEL_OVERLAY_SELECT,
    LABEL_OVERLAY_MODE,
    LABEL_SHOW_FPS,

    DISPLAY_SIZE_1X,
    DISPLAY_SIZE_2X,
    DISPLAY_SIZE_3X,
    DISPLAY_SIZE_FULL,

    ASPECT_RATIO_BY_GAME_RESOLUTION,
    ASPECT_RATIO_BY_DEV_SCREEN,
    ASPECT_RATIO_8_7,
    ASPECT_RATIO_4_3,
    ASPECT_RATIO_3_2,
    ASPECT_RATIO_16_9,

    DISPLAY_ROTATE_CW_90,
    DISPLAY_ROTATE_CW_180,
    DISPLAY_ROTATE_CW_270,

    SHADER_LCD3X,
    SHADER_SHARP_BILINEAR_SIMPLE,
    SHADER_SHARP_BILINEAR,
    SHADER_ADVANCED_AA,

    OVERLAY_MODE_OVERLAY,
    OVERLAY_MODE_BACKGROUND,

    // Menu control
    LABEL_CTRL_PLAYER,
    LABEL_FRONT_TOUCH_TO_BUTTON,
    LABEL_BACK_TOUCH_TO_BUTTON,
    LABEL_TURBO_DELAY,
    LABEL_RESET_CONFIGS_H,
    LABEL_RESET_CONFIGS_V,

    FOLLOW_DPAD,

    // Menu misc
    LABEL_AUTO_SAVE_LOAD_STATE,
    LABEL_SAVE_SCREENSHOT,
    LABEL_SAVE_PREVIEW,
    LABEL_HK_SAVESTATE,
    LABEL_HK_LOADSTATE,
    LABEL_HK_SPEED_UP,
    LABEL_HK_SPEED_DOWN,
    LABEL_HK_PLAYER_UP,
    LABEL_HK_PLAYER_DOWN,
    LABEL_HK_EXIT_GAME,

    // Menu app
    LABEL_PREVIEW_PATH,
    LABEL_PREVIEW_STYLE,
    LABEL_APP_LOG,
    LABEL_CORE_LOG,
    LABEL_SHOW_LOG,
    LABEL_LANGUAGE,

    FROM_AUTO_STATE,

    FULL_PRESERVE,
    FULL_STRETCH,
    FULL_4_3,
    FULL_3_2,

    // Menu state
    LABEL_EXISTENT_STATE,
    LABEL_NON_EXISTENT_STATE,

    LOAD,
    SAVE,
    DELETE,

    LANGUAGE_CONTAINER_SIZE,
};

typedef struct LangEntry
{
    char *name;
    char **container;
} LangEntry;

extern LangEntry lang_entries[];
extern char **cur_lang;

int GetLangsLength();
int GetLangValue(int fake_id);
int SetCurrentLang(int fake_id);

#endif