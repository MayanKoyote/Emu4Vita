#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "gui/gui.h"
#include "setting_types.h"
#include "setting_option.h"
#include "config.h"
#include "utils.h"

typedef struct OptionSlidingMenuData
{
    SettingMenu *menu;
    SettingMenuItem *menu_item;
    int id;
} OptionSlidingMenuData;

//--------------------------------------------------------------------------------------------------------
//                          String array option
//--------------------------------------------------------------------------------------------------------
static int onStrArrayOptionSlidingMenuItemClick(SlidingMenu *slidingMenu, int which)
{
    if (!slidingMenu)
        return -1;

    OptionSlidingMenuData *data = (OptionSlidingMenuData *)SlidingMenu_GetData(slidingMenu);
    if (!data || !data->menu_item)
        goto EXIT;

    StrArrayOption *option = (StrArrayOption *)data->menu_item->option_data;
    if (!option || !option->value)
        goto EXIT;

    *option->value = which;
    Setting_onStrArrayOptionItemUpdate(data->menu, data->menu_item, data->id);

    if (data->menu)
        data->menu->option_changed = 1;

    if (data->menu_item->onOptionChanged)
        data->menu_item->onOptionChanged(data->menu, data->menu_item, data->id);

EXIT:
    // SlidingMenu_Dismiss(slidingMenu);

    return 0;
}

int Setting_onStrArrayOptionItemClick(SettingMenu *menu, SettingMenuItem *menu_item, int id)
{
    if (!menu_item)
        return -1;

    StrArrayOption *option = (StrArrayOption *)menu_item->option_data;
    if (!option || !option->names)
        return -1;

    OptionSlidingMenuData *data = (OptionSlidingMenuData *)calloc(1, sizeof(OptionSlidingMenuData));
    if (!data)
        return -1;
    data->menu = menu;
    data->menu_item = menu_item;
    data->id = id;

    int n_names = option->n_names;
    char **names = calloc(n_names, sizeof(char *));
    if (!names)
    {
        free(data);
        return -1;
    }

    int focus_pos = option->value ? *option->value : 0;

    int i;
    for (i = 0; i < n_names; i++)
        names[i] = GetLangString(&option->names[i]);

    SlidingMenu *slidingMenu = SlidingMenu_Create();
    if (!slidingMenu)
    {
        free(data);
        free(names);
        return -1;
    }

    SlidingMenu_SetData(slidingMenu, data);
    SlidingMenu_SetMode(slidingMenu, TYPE_SLIDING_MENU_MODE_RIGHT);
    SlidingMenu_SetFreeDataCallback(slidingMenu, free);
    SlidingMenu_SetItems(slidingMenu, names, option->n_names);
    SlidingMenu_SetChoiceType(slidingMenu, TYPE_SLIDING_MENU_CHOICE_SINGLE);
    SlidingMenu_SetOnItemClickListener(slidingMenu, onStrArrayOptionSlidingMenuItemClick);
    SlidingMenu_SetFocusPos(slidingMenu, focus_pos);
    SlidingMenu_SetItemSeclected(slidingMenu, focus_pos, 1);
    SlidingMenu_Show(slidingMenu);
    free(names);

    return 0;
}

int Setting_onStrArrayOptionItemUpdate(SettingMenu *menu, SettingMenuItem *menu_item, int id)
{
    if (!menu_item)
        return -1;

    menu_item->option_name.lang = LANG_NULL;
    if (menu_item->option_name.string)
        menu_item->option_name.string[0] = '\0';

    StrArrayOption *option = (StrArrayOption *)menu_item->option_data;
    if (!option || !option->value || !option->names)
        return -1;

    if (*option->value > option->n_names - 1)
        *option->value = option->n_names - 1;

    menu_item->option_name.lang = option->names[*option->value].lang;
    if (option->names[*option->value].string)
    {
        if (!menu_item->option_name.string)
        {
            menu_item->option_name.string = (char *)malloc(MAX_SETTING_NAME_LENGTH);
            if (!menu_item->option_name.string)
                return -1;
        }
        snprintf(menu_item->option_name.string, MAX_SETTING_NAME_LENGTH, "%s", option->names[*option->value].string);
    }

    return 0;
}

int Setting_onStrArrayOptionClean(void *option_data)
{
    StrArrayOption *option = (StrArrayOption *)option_data;
    if (option)
    {
        if (option->names)
        {
            int i;
            for (i = 0; i < option->n_names; i++)
            {
                if (option->names[i].string)
                    free(option->names[i].string);
            }
            free(option->names);
        }
        option->names = NULL;
        option->n_names = 0;
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------------
//                          Integer array option
//--------------------------------------------------------------------------------------------------------
static int onIntArrayOptionSlidingMenuItemClick(SlidingMenu *slidingMenu, int which)
{
    if (!slidingMenu)
        return -1;

    OptionSlidingMenuData *data = (OptionSlidingMenuData *)SlidingMenu_GetData(slidingMenu);
    if (!data || !data->menu_item)
        goto EXIT;

    IntArrayOption *option = (IntArrayOption *)data->menu_item->option_data;
    if (!option || !option->value)
        goto EXIT;

    *option->value = which;
    Setting_onIntArrayOptionItemUpdate(data->menu, data->menu_item, data->id);

    if (data->menu)
        data->menu->option_changed = 1;

    if (data->menu_item->onOptionChanged)
        data->menu_item->onOptionChanged(data->menu, data->menu_item, data->id);

EXIT:
    // SlidingMenu_Dismiss(slidingMenu);

    return 0;
}

int Setting_onIntArrayOptionItemClick(SettingMenu *menu, SettingMenuItem *menu_item, int id)
{
    if (!menu_item)
        return -1;

    IntArrayOption *option = (IntArrayOption *)menu_item->option_data;
    if (!option || !option->values || !option->format)
        return -1;

    OptionSlidingMenuData *data = (OptionSlidingMenuData *)calloc(1, sizeof(OptionSlidingMenuData));
    if (!data)
        return -1;
    data->menu = menu;
    data->menu_item = menu_item;
    data->id = id;

    int n_names = option->n_values;
    char **names = calloc(n_names, sizeof(char *));
    if (!names)
    {
        free(data);
        return -1;
    }

    int focus_pos = 0;

    int i;
    for (i = 0; i < n_names; i++)
    {
        if (option->value && *option->value == option->values[i])
            focus_pos = i;

        names[i] = (char *)malloc(MAX_SETTING_NAME_LENGTH);
        if (names[i])
            snprintf(names[i], MAX_SETTING_NAME_LENGTH, option->format, option->values[i]);
    }

    SlidingMenu *slidingMenu = SlidingMenu_Create();
    if (!slidingMenu)
    {
        free(data);
        free(names);
        return -1;
    }

    SlidingMenu_SetData(slidingMenu, data);
    SlidingMenu_SetMode(slidingMenu, TYPE_SLIDING_MENU_MODE_RIGHT);
    SlidingMenu_SetFreeDataCallback(slidingMenu, free);
    SlidingMenu_SetItems(slidingMenu, names, n_names);
    SlidingMenu_SetChoiceType(slidingMenu, TYPE_SLIDING_MENU_CHOICE_SINGLE);
    SlidingMenu_SetOnItemClickListener(slidingMenu, onIntArrayOptionSlidingMenuItemClick);
    SlidingMenu_SetFocusPos(slidingMenu, focus_pos);
    SlidingMenu_SetItemSeclected(slidingMenu, focus_pos, 1);
    SlidingMenu_Show(slidingMenu);

    for (i = 0; i < n_names; i++)
    {
        if (names[i])
            free(names[i]);
    }
    free(names);

    return 0;
}

int Setting_onIntArrayOptionItemUpdate(SettingMenu *menu, SettingMenuItem *menu_item, int id)
{
    if (!menu_item)
        return -1;

    menu_item->option_name.lang = LANG_NULL;
    if (!menu_item->option_name.string)
    {
        menu_item->option_name.string = (char *)malloc(MAX_SETTING_NAME_LENGTH);
        if (!menu_item->option_name.string)
            return -1;
    }
    menu_item->option_name.string[0] = '\0';

    IntArrayOption *option = (IntArrayOption *)menu_item->option_data;
    if (!option || !option->value || !option->values || !option->format)
        return -1;

    if (*option->value > option->n_values - 1)
        *option->value = option->n_values - 1;

    snprintf(menu_item->option_name.string, MAX_SETTING_NAME_LENGTH, option->format, option->values[*option->value]);

    return 0;
}

int Setting_onIntArrayOptionClean(void *option_data)
{
    IntArrayOption *option = (IntArrayOption *)option_data;
    if (option)
    {
        if (option->format)
            free(option->format);
        option->format = NULL;

        if (option->values)
            free(option->values);
        option->values = NULL;
        option->n_values = 0;
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------------
//                          Integer range option
//--------------------------------------------------------------------------------------------------------
static int onIntRangeOptionSlidingMenuItemClick(SlidingMenu *slidingMenu, int which)
{
    if (!slidingMenu)
        return -1;

    OptionSlidingMenuData *data = (OptionSlidingMenuData *)SlidingMenu_GetData(slidingMenu);
    if (!data || !data->menu_item)
        goto EXIT;

    IntRangeOption *option = (IntRangeOption *)data->menu_item->option_data;
    if (!option || !option->value)
        goto EXIT;

    *option->value = MIN(option->min + which * option->step, option->max);
    Setting_onIntRangeOptionItemUpdate(data->menu, data->menu_item, data->id);

    if (data->menu)
        data->menu->option_changed = 1;

    if (data->menu_item->onOptionChanged)
        data->menu_item->onOptionChanged(data->menu, data->menu_item, data->id);

EXIT:
    // SlidingMenu_Dismiss(slidingMenu);

    return 0;
}

int Setting_onIntRangeOptionItemClick(SettingMenu *menu, SettingMenuItem *menu_item, int id)
{
    if (!menu_item)
        return -1;

    IntRangeOption *option = (IntRangeOption *)menu_item->option_data;
    if (!option || !option->format)
        return -1;

    OptionSlidingMenuData *data = (OptionSlidingMenuData *)calloc(1, sizeof(OptionSlidingMenuData));
    if (!data)
        return -1;
    data->menu = menu;
    data->menu_item = menu_item;
    data->id = id;

    int n_names = (option->max - option->min) / option->step + 1;
    if ((option->max - option->min) % option->step != 0) // 无法整除时需多添加一位，否则最大选项到不了max
        n_names++;

    char **names = calloc(n_names, sizeof(char *));
    if (!names)
    {
        free(data);
        return -1;
    }

    int focus_pos = 0;

    int value = option->min;
    int i;
    for (i = 0; i < n_names; i++)
    {
        if (option->value && *option->value == value)
            focus_pos = i;

        names[i] = (char *)malloc(MAX_SETTING_NAME_LENGTH);
        if (names[i])
            snprintf(names[i], MAX_SETTING_NAME_LENGTH, option->format, value);

        value = MIN(value + option->step, option->max);
    }

    SlidingMenu *slidingMenu = SlidingMenu_Create();
    if (!slidingMenu)
    {
        free(data);
        free(names);
        return -1;
    }

    SlidingMenu_SetData(slidingMenu, data);
    SlidingMenu_SetMode(slidingMenu, TYPE_SLIDING_MENU_MODE_RIGHT);
    SlidingMenu_SetFreeDataCallback(slidingMenu, free);
    SlidingMenu_SetItems(slidingMenu, names, n_names);
    SlidingMenu_SetChoiceType(slidingMenu, TYPE_SLIDING_MENU_CHOICE_SINGLE);
    SlidingMenu_SetOnItemClickListener(slidingMenu, onIntRangeOptionSlidingMenuItemClick);
    SlidingMenu_SetFocusPos(slidingMenu, focus_pos);
    SlidingMenu_SetItemSeclected(slidingMenu, focus_pos, 1);
    SlidingMenu_Show(slidingMenu);

    for (i = 0; i < n_names; i++)
    {
        if (names[i])
            free(names[i]);
    }
    free(names);

    return 0;
}

int Setting_onIntRangeOptionItemUpdate(SettingMenu *menu, SettingMenuItem *menu_item, int id)
{
    if (!menu_item)
        return -1;

    menu_item->option_name.lang = LANG_NULL;
    if (!menu_item->option_name.string)
    {
        menu_item->option_name.string = (char *)malloc(MAX_SETTING_NAME_LENGTH);
        if (!menu_item->option_name.string)
            return -1;
    }
    menu_item->option_name.string[0] = '\0';

    IntRangeOption *option = (IntRangeOption *)menu_item->option_data;
    if (!option || !option->value || !option->format)
        return -1;

    if (*option->value > option->max)
        *option->value = option->max;
    if (*option->value < option->min)
        *option->value = option->min;

    snprintf(menu_item->option_name.string, MAX_SETTING_NAME_LENGTH, option->format, *option->value);

    return 0;
}

int Setting_onIntRangeOptionClean(void *option_data)
{
    IntRangeOption *option = (IntRangeOption *)option_data;
    if (option)
    {
        if (option->format)
            free(option->format);
        option->format = NULL;
        option->min = 0;
        option->max = 0;
        option->step = 0;
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------------
//                          Key map option
//--------------------------------------------------------------------------------------------------------
static int onKeyMapOptionSlidingMenuItemClick(SlidingMenu *slidingMenu, int which)
{
    if (!slidingMenu)
        return -1;

    OptionSlidingMenuData *data = (OptionSlidingMenuData *)SlidingMenu_GetData(slidingMenu);
    if (!data || !data->menu_item)
        return -1;

    KeyMapOption *option = (KeyMapOption *)data->menu_item->option_data;
    if (!option || !option->value)
        return -1;

    int lenghth = SlidingMenu_GetItemsLength(slidingMenu);
    uint32_t map_key = 0;
    int i;
    for (i = 0; i < option->n_entries && i < lenghth; i++)
    {
        if (SlidingMenu_IsItemSeclected(slidingMenu, i))
            map_key |= option->entries[i].key;
    }

    *option->value = map_key;
    Setting_onKeyMapOptionItemUpdate(data->menu, data->menu_item, data->id);

    if (data->menu)
        data->menu->option_changed = 1;

    if (data->menu_item->onOptionChanged)
        data->menu_item->onOptionChanged(data->menu, data->menu_item, data->id);

    return 0;
}

static int onKeyMapOptionSlidingMenuSelectChanged(SlidingMenu *slidingMenu)
{
    return onKeyMapOptionSlidingMenuItemClick(slidingMenu, 0);
}

int Setting_OnKeyMapOptionItemClick(SettingMenu *menu, SettingMenuItem *menu_item, int id)
{
    if (!menu_item)
        return -1;

    KeyMapOption *option = (KeyMapOption *)menu_item->option_data;
    if (!option)
        return -1;

    OptionSlidingMenuData *data = (OptionSlidingMenuData *)calloc(1, sizeof(OptionSlidingMenuData));
    if (!data)
        return -1;
    data->menu = menu;
    data->menu_item = menu_item;
    data->id = id;

    int n_names = option->n_entries;
    char **names = calloc(n_names, sizeof(char *));
    if (!names)
    {
        free(data);
        return -1;
    }

    int i;
    for (i = 0; i < n_names; i++)
        names[i] = GetLangString(&option->entries[i].name);

    SlidingMenu *slidingMenu = SlidingMenu_Create();
    if (!slidingMenu)
    {
        free(data);
        free(names);
        return -1;
    }

    SlidingMenu_SetData(slidingMenu, data);
    SlidingMenu_SetMode(slidingMenu, TYPE_SLIDING_MENU_MODE_RIGHT);
    SlidingMenu_SetFreeDataCallback(slidingMenu, free);
    SlidingMenu_SetItems(slidingMenu, names, n_names);
    SlidingMenu_SetChoiceType(slidingMenu, TYPE_SLIDING_MENU_CHOICE_MULTIPLE);
    SlidingMenu_SetOnItemClickListener(slidingMenu, onKeyMapOptionSlidingMenuItemClick);
    SlidingMenu_SetOnSelectChangedListener(slidingMenu, onKeyMapOptionSlidingMenuSelectChanged);

    if (option->value) // 设置选中
    {
        for (i = 0; i < n_names; i++)
        {
            if ((*option->value & option->entries[i].key) == option->entries[i].key)
                SlidingMenu_SetItemSeclected(slidingMenu, i, 1);
        }
    }

    SlidingMenu_Show(slidingMenu);
    free(names);

    return 0;
}

int Setting_onKeyMapOptionItemUpdate(SettingMenu *menu, SettingMenuItem *menu_item, int id)
{
    if (!menu_item)
        return -1;

    menu_item->option_name.lang = LANG_NONE;
    if (!menu_item->option_name.string)
    {
        menu_item->option_name.string = (char *)malloc(MAX_SETTING_NAME_LENGTH);
        if (!menu_item->option_name.string)
            return -1;
    }
    menu_item->option_name.string[0] = '\0';

    KeyMapOption *option = (KeyMapOption *)menu_item->option_data;
    if (!option || !option->value)
        return -1;

    const char *map_name = NULL;
    uint32_t map_key = *option->value;
    int n_mapped = 0;

    int i;
    for (i = 0; i < option->n_entries; i++)
    {
        if ((map_key & option->entries[i].key) == option->entries[i].key)
        {
            if (option->entries[i].name.lang != LANG_TURBO) // 跳过连发按键，之后会在后面加上
            {
                map_name = GetLangString(&option->entries[i].name);
                if (map_name)
                {
                    if (n_mapped > 0)
                        strcat(menu_item->option_name.string, "+");
                    strcat(menu_item->option_name.string, map_name);
                }
                n_mapped++;
            }
        }
    }

    if (map_key & TURBO_BITMASK_KEY)
    {
        int len = strlen(menu_item->option_name.string);
        char *p = menu_item->option_name.string + len;
        snprintf(p, MAX_SETTING_NAME_LENGTH - len, " [%s]", cur_lang[LANG_TURBO]);
    }

    if (menu_item->option_name.string[0] != '\0')
        menu_item->option_name.lang = LANG_NULL;

    return 0;
}

int Setting_onKeyMapOptionClean(void *option_data)
{
    KeyMapOption *option = (KeyMapOption *)option_data;
    if (option)
    {
        if (option->entries)
        {
            int i;
            for (i = 0; i < option->n_entries; i++)
            {
                if (option->entries[i].name.string)
                    free(option->entries[i].name.string);
            }
            free(option->entries);
        }
        option->entries = NULL;
        option->n_entries = 0;
    }

    return 0;
}
