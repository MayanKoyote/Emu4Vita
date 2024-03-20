#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <psp2/io/dirent.h>
#include <psp2/io/fcntl.h>

#include "list/overlay_list.h"
#include "list/option_list.h"
#include "list/cheat_list.h"
#include "activity/activity.h"
#include "emu/emu.h"
#include "gui/gui.h"
#include "setting.h"
#include "setting_menu.h"
#include "setting_callbacks.h"
#include "setting_option.h"
#include "setting_overlay.h"
#include "setting_window.h"
#include "config.h"
#include "utils.h"
#include "lang.h"
#include "init.h"
#include "boot.h"

static SettingWindow *setting_window = NULL;

int setting_resume_game_enabled = 0;
int setting_config_type = 0;
int setting_has_main_core_menu = 0;
int setting_game_is_loaded = 0;
int setting_current_path_is_file = 0;

int setting_visibility_core_menu = 0;
int setting_visibility_cheat_menu = 0;
int setting_visibility_app_menu = 0;
int setting_visibility_disk_control_item = 0;
int setting_visibility_exit_to_arch_item = 0;
int setting_visibility_touch_to_button_item = 0;

uint32_t setting_language_config_value = 0;

extern SettingContext setting_context;

static void updateVariables()
{
    setting_resume_game_enabled = 1;
    setting_game_is_loaded = Emu_IsGameLoaded();
    setting_config_type = setting_game_is_loaded ? TYPE_CONFIG_GAME : TYPE_CONFIG_MAIN;
    setting_current_path_is_file = CurrentPathIsFile();
    setting_visibility_app_menu = !setting_game_is_loaded;
    setting_visibility_core_menu = (setting_context.menus[ID_SETTING_MENU_CORE].items && (setting_game_is_loaded || setting_has_main_core_menu));
    setting_visibility_cheat_menu = (setting_context.menus[ID_SETTING_MENU_CHEAT].items && setting_game_is_loaded);
    setting_visibility_disk_control_item = (setting_game_is_loaded && Emu_HasDiskControl() && Emu_DiskGetNumImages() > 1);
    setting_visibility_exit_to_arch_item = (BootGetMode() == BOOT_MODE_ARCH);
    setting_visibility_touch_to_button_item = !is_vitatv_model;
}

int Setting_UpdateMenu(SettingMenu *menu)
{
    if (!menu)
        return -1;

    if (menu->items)
    {
        int i;
        for (i = 0; i < menu->n_items; i++)
        {
            if (menu->items[i].onItemUpdate)
                menu->items[i].onItemUpdate(menu, &menu->items[i], i);
        }
    }

    return 0;
}

int Setting_CleanMenu(SettingMenu *menu)
{
    if (!menu)
        return -1;

    int i;
    for (i = 0; i < menu->n_items; i++)
    {
        if (menu->items[i].option_data && menu->items[i].onOptionClean)
        {
            menu->items[i].onOptionClean(menu->items[i].option_data);
            free(menu->items[i].option_data);
            menu->items[i].option_data = NULL;
        }
    }
    menu->items = NULL;
    menu->n_items = 0;

    if (menu->name.string)
    {
        free(menu->name.string);
        menu->name.string = NULL;
    }

    return 0;
}

int Setting_SetCoreMenu(LinkedList *list)
{
    SettingMenu *menu = &setting_context.menus[ID_SETTING_MENU_CORE];
    if (!menu)
        return -1;

    // Clean old menu
    Setting_CleanMenu(menu);

    if (!list)
        return -1;

    int l_length = LinkedListGetLength(list);

    // Create new menu items
    int n_items = l_length + 1; // +1 for reset config
    SettingMenuItem *items = (SettingMenuItem *)calloc(n_items, sizeof(SettingMenuItem));
    if (!items)
        return -1;

    LinkedListEntry *entry = LinkedListHead(list);

    int i, j;
    for (i = 0; i < l_length && entry; i++)
    {
        OptionListEntryData *data = (OptionListEntryData *)LinkedListGetEntryData(entry);

        // Disable use lang
        items[i].name.lang = LANG_NULL;
        items[i].visibility = &(data->visibility); // 可见性由retro控制，传递指针进来

        // Item name
        char *desc = data->desc;
        if (!desc)
            desc = data->key;
        if (desc)
        {
            // printf("core_option: name: %s\n", desc);
            items[i].name.string = (char *)malloc(strlen(desc) + 1);
            if (items[i].name.string)
                strcpy(items[i].name.string, desc);
        }

        if (!data->values || data->n_values <= 0)
            continue;

        // Item option
        StrArrayOption *option = (StrArrayOption *)calloc(1, sizeof(StrArrayOption));
        if (!option)
            continue;
        items[i].option_data = option;

        // Item callbacks
        items[i].onItemClick = Setting_onStrArrayOptionItemClick;
        items[i].onItemUpdate = Setting_onStrArrayOptionItemUpdate;
        items[i].onOptionChanged = Setting_onCoreMenuOptionChanged;
        items[i].onOptionClean = Setting_onStrArrayOptionClean;

        // Item option value
        option->value = &(data->select);

        // Item option names
        option->names = (LangString *)calloc(data->n_values, sizeof(LangString));
        if (!option->names)
            continue;
        option->n_names = data->n_values;

        for (j = 0; j < option->n_names; j++)
        {
            // Disable use lang
            option->names[j].lang = LANG_NULL;

            char *name = data->values[j].label;
            if (!name)
                name = data->values[j].value;
            if (name)
            {
                option->names[j].string = (char *)malloc(strlen(name) + 1);
                if (option->names[j].string)
                    strcpy(option->names[j].string, name);
            }
        }

        entry = LinkedListNext(entry);
    }

    // The last one is reset config
    items[n_items - 1].name.lang = LANG_RESET_CONFIGS;
    items[n_items - 1].onItemClick = Setting_onResetCoreConfigItemClick;
    items[n_items - 1].visibility = NULL;

    menu->items = items;
    menu->n_items = n_items;

    return 0;
}

int Setting_SetCheatMenu(LinkedList *list)
{
    SettingMenu *menu = &setting_context.menus[ID_SETTING_MENU_CHEAT];
    if (!menu)
        return -1;

    // Clean old menu
    Setting_CleanMenu(menu);

    if (!list)
        return -1;

    int l_length = LinkedListGetLength(list);

    // Create new menu items
    int n_items = l_length + 1; // +1 for reset config
    SettingMenuItem *items = (SettingMenuItem *)calloc(n_items, sizeof(SettingMenuItem));
    if (!items)
        return -1;

    LinkedListEntry *entry = LinkedListHead(list);

    int i;
    for (i = 0; i < l_length && entry; i++)
    {
        CheatListEntryData *data = (CheatListEntryData *)LinkedListGetEntryData(entry);

        // Disable use lang
        items[i].name.lang = LANG_NULL;
        items[i].visibility = NULL;

        // Item name
        char *desc = data->desc;
        if (desc)
        {
            // printf("cheat_option: name: %s\n", desc);
            items[i].name.string = (char *)malloc(strlen(desc) + 1);
            if (items[i].name.string)
                strcpy(items[i].name.string, desc);
        }

        // Item option data
        StrArrayOption *option = (StrArrayOption *)calloc(1, sizeof(StrArrayOption));
        if (!option)
            continue;
        items[i].option_data = option;

        // Item callbacks
        items[i].onItemClick = Setting_onStrArrayOptionItemClick;
        items[i].onItemUpdate = Setting_onStrArrayOptionItemUpdate;
        items[i].onOptionClean = Setting_onStrArrayOptionClean;

        // Item option value
        option->value = &(data->enable);

        // Item option names
        option->n_names = 2;
        option->names = (LangString *)calloc(option->n_names, sizeof(LangString));
        if (!option->names)
            continue;
        option->names[0].lang = LANG_NO;
        option->names[1].lang = LANG_YES;

        entry = LinkedListNext(entry);
    }

    // The last one is reset config
    items[n_items - 1].name.lang = LANG_RESET_CONFIGS;
    items[n_items - 1].onItemClick = Setting_onResetCheatConfigItemClick;
    items[n_items - 1].visibility = NULL;

    menu->items = items;
    menu->n_items = n_items;

    return 0;
}

int Setting_SetOverlayOption(LinkedList *list)
{
    if (!list)
        return -1;

    StrArrayOption *option = Setting_GetOverlaySelectOption();
    if (!option)
        return -1;

    // Clean old option
    Setting_onStrArrayOptionClean(option);

    // Create new option
    int ls_length = LinkedListGetLength(list);
    int n_names = ls_length + 1; // +1 for none
    LangString *names = (LangString *)calloc(n_names, sizeof(LangString));
    if (!names)
        return -1;

    names[0].lang = LANG_NONE; // The first one is none

    LinkedListEntry *entry = LinkedListHead(list);

    int i;
    for (i = 1; i < n_names && entry; i++)
    {
        names[i].lang = LANG_NULL;

        OverlayListEntryData *data = (OverlayListEntryData *)LinkedListGetEntryData(entry);
        if (data->name)
        {
            // printf("overlay_option: name = %s\n", data->name);
            names[i].string = (char *)malloc(strlen(data->name) + 1);
            if (names[i].string)
                strcpy(names[i].string, data->name);
        }
        entry = LinkedListNext(entry);
    }

    option->names = names;
    option->n_names = n_names;

    if (graphics_config.overlay_select > n_names - 1)
        graphics_config.overlay_select = 0;

    return 0;
}

int Setting_SetLangOption(LangEntry *entries, int n_entries)
{
    StrArrayOption *option = Setting_GetLangOption();
    if (!option)
        return -1;

    // Clean old option
    Setting_onStrArrayOptionClean(option);

    // Create new option
    int n_names = 0;
    int i;

    for (i = 0; i < n_entries; i++)
    {
        if (entries[i].container)
            n_names++;
    }

    LangString *names = (LangString *)calloc(n_names, sizeof(LangString));
    if (!names)
        return -1;

    int names_idx = 0;
    for (i = 0; i < n_entries && names_idx < n_names; i++)
    {
        if (entries[i].container)
        {
            names[names_idx].lang = LANG_NULL;
            if (entries[i].name)
            {
                // printf("lang_option: name = %s\n", entries[i].name);
                names[names_idx].string = (char *)malloc(strlen(entries[i].name) + 1);
                if (names[names_idx].string)
                    strcpy(names[names_idx].string, entries[i].name);
            }
            names_idx++;
        }
    }

    option->names = names;
    option->n_names = n_names;

    setting_language_config_value = GetConfigValueByLangIndex(app_config.language);
    if (setting_language_config_value > n_names - 1)
    {
        setting_language_config_value = 0;
        app_config.language = GetLangIndexByConfigValue(setting_language_config_value);
    }
    SetCurrentLang(app_config.language);

    return 0;
}

int Setting_Init()
{
    // 判断core_menu的列表是否存在，有的话说明在libretro初始化时便有了core_menu，则它是具有全局性的
    if (setting_context.menus[ID_SETTING_MENU_CORE].items)
        setting_has_main_core_menu = 1;
    Setting_SetLangOption(GetLangEntries(), GetLangEntriesLength()); // 语言条目因为有缺失，只显示存在的语言，需要初始化
    Setting_InitOverlay();                                           // Overlay的设置选项是从文本里获取的，需要初始化
    if (!setting_window)
    {
        setting_window = Setting_CreateWindow(&setting_context);
        if (!setting_window)
            return -1;
    }
    Setting_SetWindowAutoFree(setting_window, 0);
    Setting_SetWindowContext(setting_window, &setting_context);

    return 0;
}

int Setting_Deinit()
{
    if (setting_window)
    {
        Setting_SetWindowAutoFree(setting_window, 1);
        Setting_CloseWindow(setting_window);
        setting_window = NULL;
    }
    Setting_DeinitOverlay();
    return 0;
}

int Setting_OpenMenu()
{
    Setting_WaitOverlayInitEnd();
    updateVariables();
    Setting_OpenWindow(setting_window);

    return 0;
}

int Setting_CloseMenu()
{
    Setting_CloseWindow(setting_window);

    return 0;
}

int Setting_IsResumeGameEnabled()
{
    return setting_resume_game_enabled;
}
