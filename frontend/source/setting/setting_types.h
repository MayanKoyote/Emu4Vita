#ifndef __M_SETTING_TYPES_H__
#define __M_SETTING_TYPES_H__

#include <stdint.h>

#include "lang.h"

#define MAX_SETTING_NAME_LENGTH 256

typedef enum SettingMenuId
{
    ID_SETTING_MENU_MAIN,
    ID_SETTING_MENU_STATE,
    ID_SETTING_MENU_GRAPHICS,
    ID_SETTING_MENU_CONTROL,
    ID_SETTING_MENU_HOTKEY,
    ID_SETTING_MENU_CORE,
    ID_SETTING_MENU_CHEAT,
    ID_SETTING_MENU_MISC,
    ID_SETTING_MENU_APP,
} SettingMenuId;

typedef struct SettingContext SettingContext;
typedef struct SettingMenu SettingMenu;
typedef struct SettingMenuItem SettingMenuItem;

typedef struct
{
    LangString name;
    uint32_t key;
} KeyMapEntry;

typedef struct
{
    uint32_t *value;
    KeyMapEntry *entries;
    int n_entries;
} KeyMapOption;

typedef struct
{
    uint32_t *value;
    LangString *names;
    int n_names;
} StrArrayOption;

typedef struct
{
    uint32_t *value;
    int *values;
    int n_values;
    char *format;
} IntArrayOption;

typedef struct
{
    int32_t *value;
    int32_t min;
    int32_t max;
    int32_t step;
    char *format;
} IntRangeOption;

struct SettingMenuItem
{
    LangString name;                                                               // 条目名
    LangString option_name;                                                        // 要显示的设置名 (一般由onOptionUpdate设置)
    void *option_data;                                                             // 设置数据
    int *visibility;                                                               // 可见性 (指针设为NULL时为默认可见)
    int (*onItemClick)(SettingMenu *menu, SettingMenuItem *menu_item, int id);     // Item点击事件
    int (*onItemUpdate)(SettingMenu *menu, SettingMenuItem *menu_item, int id);    // Item更新事件
    int (*onOptionChanged)(SettingMenu *menu, SettingMenuItem *menu_item, int id); // Option变更事件
    int (*onOptionClean)(void *option_data);                                       // option_data清空事件
};

struct SettingMenu
{
    LangString name;
    SettingMenuItem *items;
    int n_items;
    int *visibility;
    int menu_pos;
    int option_changed;
    int (*onStart)(SettingMenu *menu);
    int (*onFinish)(SettingMenu *menu);
    int (*onExit)(SettingMenu *menu);
    int (*onDraw)(SettingMenu *menu);
    int (*onCtrl)(SettingMenu *menu);
};

struct SettingContext
{
    SettingMenu *menus;
    int n_menus;
    int menus_pos;
};

#define SETTING_IS_VISIBLE(visibility) (visibility ? *visibility : 1)

#define STRARRAY_OPTION_ITEM(lang, option_data, visibility, onOptionChanged)                                                                                   \
    {                                                                                                                                                          \
        {lang, NULL}, {LANG_NULL, NULL}, option_data, visibility, Setting_onStrArrayOptionItemClick, Setting_onStrArrayOptionItemUpdate, onOptionChanged, NULL \
    }

#define INTARRAY_OPTION_ITEM(lang, option_data, visibility, onOptionChanged)                                                                                   \
    {                                                                                                                                                          \
        {lang, NULL}, {LANG_NULL, NULL}, option_data, visibility, Setting_onIntArrayOptionItemClick, Setting_onIntArrayOptionItemUpdate, onOptionChanged, NULL \
    }

#define INTRANGE_OPTION_ITEM(lang, option_data, visibility, onOptionChanged)                                                                                   \
    {                                                                                                                                                          \
        {lang, NULL}, {LANG_NULL, NULL}, option_data, visibility, Setting_onIntRangeOptionItemClick, Setting_onIntRangeOptionItemUpdate, onOptionChanged, NULL \
    }

#define KEYMAP_OPTION_ITEM(lang, option_data, visibility, onOptionChanged)                                                                                 \
    {                                                                                                                                                      \
        {lang, NULL}, {LANG_NULL, NULL}, option_data, visibility, Setting_OnKeyMapOptionItemClick, Setting_onKeyMapOptionItemUpdate, onOptionChanged, NULL \
    }

#define FUCNTION_ITEM(lang, visibility, onItemClick)                                     \
    {                                                                                    \
        {lang, NULL}, {LANG_NULL, NULL}, NULL, visibility, onItemClick, NULL, NULL, NULL \
    }

#define CUSTOM_OPTION_ITEM(lang, option_data, visibility, onItemClick, onItemUpdate, onOptionChanged, onOptionClean)        \
    {                                                                                                                       \
        {lang, NULL}, {LANG_NULL, NULL}, option_data, visibility, onItemClick, onItemUpdate, onOptionChanged, onOptionClean \
    }

#define SIMPLE_SETTING_MENU(lang, items, n_items, visibility, onStart, onFinish, onExit)      \
    {                                                                                         \
        {lang, NULL}, items, n_items, visibility, 0, 0, onStart, onFinish, onExit, NULL, NULL \
    }

#define CUSTOM_SETTING_MENU(lang, items, n_items, visibility, onStart, onFinish, onExit, onDraw, onCtrl) \
    {                                                                                                    \
        {lang, NULL}, items, n_items, visibility, 0, 0, onStart, onFinish, onExit, onDraw, onCtrl         \
    }

#endif
