#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "activity/activity.h"
#include "emu/emu.h"
#include "setting.h"
#include "config.h"
#include "boot.h"
#include "init.h"
#include "lang.h"
#include "file.h"

#include "setting_types.h"
#include "setting_menu.h"
#include "setting_option.h"

extern int setting_config_type;
extern int setting_resume_game_enabled;
extern uint32_t setting_language_config_value;

extern int Setting_UpdateMenu(SettingMenu *menu);

//--------------------------------------------------------------------------------------------------------
//                          Main menu callbacks
//--------------------------------------------------------------------------------------------------------
int Setting_onResumeGameItemClick(SettingMenu *menu, SettingMenuItem *menu_item, int id)
{
    Setting_CloseMenu();
    return 0;
}

int Setting_onResetGameItemClick(SettingMenu *menu, SettingMenuItem *menu_item, int id)
{
    Setting_CloseMenu();
    Emu_ResetGame();
    return 0;
}

int Setting_onExitGameItemClick(SettingMenu *menu, SettingMenuItem *menu_item, int id)
{
    setting_resume_game_enabled = 0;
    Setting_CloseMenu();
    Emu_ExitGame();
    if (BootGetMode() == BOOT_MODE_GAME)
        BootLoadParentExec();
    return 0;
}

static int onDiskControlAlertDialogPositiveClick(AlertDialog *dialog, int which)
{
    Setting_CloseMenu();
    Emu_DiskChangeImageIndex(which);
    AlertDialog_Dismiss(dialog);
    return 0;
}

int Setting_onDiskControlItemClick(SettingMenu *menu, SettingMenuItem *menu_item, int id)
{
    AlertDialog *dialog = AlertDialog_Create();
    if (!dialog)
        return -1;

    int n_items = Emu_DiskGetNumImages();
    char **items = (char **)malloc(n_items * sizeof(char *));
    if (!items)
    {
        AlertDialog_Destroy(dialog);
        return -1;
    }

    int cur_index = Emu_DiskGetImageIndex();
    int i;
    for (i = 0; i < n_items; i++)
    {
        items[i] = (char *)malloc(MAX_NAME_LENGTH);
        if (items[i])
        {
            if (i == cur_index)
                snprintf(items[i], MAX_NAME_LENGTH, "%s %d (%s)", cur_lang[LANG_DISK], i + 1, cur_lang[LANG_CURRENT]);
            else
                snprintf(items[i], MAX_NAME_LENGTH, "%s %d", cur_lang[LANG_DISK], i + 1);
        }
    }

    AlertDialog_SetTitle(dialog, cur_lang[LANG_SWITCH_DISK]);
    AlertDialog_SetItems(dialog, items, n_items);
    AlertDialog_SetPositiveButton(dialog, cur_lang[LANG_CONFIRM], onDiskControlAlertDialogPositiveClick);
    AlertDialog_SetNegativeButton(dialog, cur_lang[LANG_CANCEL], AlertDialog_OnClickDismiss);
    AlertDialog_Show(dialog);

    for (i = 0; i < n_items; i++)
    {
        if (items[i])
            free(items[i]);
    }
    free(items);

    return 0;
}

int Setting_onExitToArchItemClick(SettingMenu *menu, SettingMenuItem *menu_item, int id)
{
    setting_resume_game_enabled = 0;
    Setting_CloseMenu();
    Emu_ExitGame();
    BootLoadParentExec();
    return 0;
}

int Setting_onExitAppItemClick(SettingMenu *menu, SettingMenuItem *menu_item, int id)
{
    setting_resume_game_enabled = 0;
    Setting_CloseMenu();
    Emu_ExitGame();
    AppExit();
    return 0;
}

//--------------------------------------------------------------------------------------------------------
//                          State menu callbacks
//--------------------------------------------------------------------------------------------------------
int Setting_onStateMenuStart(SettingMenu *menu)
{
    Setting_InitState();
    return 0;
}

int Setting_onStateMenuFinish(SettingMenu *menu)
{
    Setting_DeinitState();
    return 0;
}

int Setting_onStateMenuDraw(SettingMenu *menu)
{
    Setting_DrawState();
    return 0;
}

int Setting_onStateMenuCtrl(SettingMenu *menu)
{
    Setting_CtrlState();
    return 0;
}

//--------------------------------------------------------------------------------------------------------
//                          Graphics menu callbacks
//--------------------------------------------------------------------------------------------------------
int Setting_onGraphicsMenuFinish(SettingMenu *menu)
{
    if (menu->option_changed)
    {
        SaveGraphicsConfig(setting_config_type);
        menu->option_changed = 0;
    }
    return 0;
}

int Setting_onGraphicsMenuOptionChanged(SettingMenu *menu, SettingMenuItem *menu_item, int id)
{
    if (Emu_IsGameLoaded())
        Emu_RequestUpdateVideoDisplay();
    return 0;
}

int Setting_onResetGraphicsConfigItemClick(SettingMenu *menu, SettingMenuItem *menu_item, int id)
{
    ResetGraphicsConfig();
    Setting_onGraphicsMenuOptionChanged(menu, NULL, 0);
    Setting_UpdateMenu(menu);
    menu->option_changed = 1;
    return 0;
}

//--------------------------------------------------------------------------------------------------------
//                          Control menu callbacks
//--------------------------------------------------------------------------------------------------------
int Setting_onControlMenuFinish(SettingMenu *menu)
{
    if (menu->option_changed)
    {
        SaveControlConfig(setting_config_type);
        menu->option_changed = 0;
    }
    return 0;
}

int Setting_onResetControlConfigItemClick(SettingMenu *menu, SettingMenuItem *menu_item, int id)
{
    ResetControlConfig();
    Setting_UpdateMenu(menu);
    menu->option_changed = 1;
    return 0;
}

//--------------------------------------------------------------------------------------------------------
//                          Hotkey menu callbacks
//--------------------------------------------------------------------------------------------------------
int Setting_onHotkeyMenuFinish(SettingMenu *menu)
{
    if (menu->option_changed)
    {
        SaveHotkeyConfig(setting_config_type);
        menu->option_changed = 0;
    }
    return 0;
}

int Setting_onResetHotkeyConfigItemClick(SettingMenu *menu, SettingMenuItem *menu_item, int id)
{
    ResetHotkeyConfig(setting_config_type);
    Setting_UpdateMenu(menu);
    menu->option_changed = 1;
    return 0;
}

//--------------------------------------------------------------------------------------------------------
//                          Core menu callbacks
//--------------------------------------------------------------------------------------------------------
int Setting_onCoreMenuFinish(SettingMenu *menu)
{
    if (menu->option_changed)
    {
        SaveCoreConfig(setting_config_type);
        Retro_RequestUpdateVariable();
        menu->option_changed = 0;
    }
    return 0;
}

int Setting_onCoreMenuOptionChanged(SettingMenu *menu, SettingMenuItem *menu_item, int id)
{
    Retro_UpdateCoreOptionsDisplay();
    Setting_UpdateMenu(menu);
    return 0;
}

int Setting_onResetCoreConfigItemClick(SettingMenu *menu, SettingMenuItem *menu_item, int id)
{
    ResetCoreConfig();
    Retro_UpdateCoreOptionsDisplay();
    Setting_UpdateMenu(menu);
    menu->option_changed = 1;
    return 0;
}

//--------------------------------------------------------------------------------------------------------
//                          Cheat menu callbacks
//--------------------------------------------------------------------------------------------------------
int Setting_onCheatMenuFinish(SettingMenu *menu)
{
    if (menu->option_changed)
    {
        Emu_SaveCheatOption();
        Emu_UpdateCheatOption();
        menu->option_changed = 1;
    }
    return 0;
}

int Setting_onResetCheatConfigItemClick(SettingMenu *menu, SettingMenuItem *menu_item, int id)
{
    Emu_ResetCheatOption();
    Setting_UpdateMenu(menu);
    menu->option_changed = 1;
    return 0;
}

//--------------------------------------------------------------------------------------------------------
//                          Misc menu callbacks
//--------------------------------------------------------------------------------------------------------
int Setting_onMiscMenuFinish(SettingMenu *menu)
{
    if (menu->option_changed)
    {
        SaveMiscConfig(setting_config_type);
        menu->option_changed = 0;
    }
    return 0;
}

int Setting_onAutoSaveLoadOptionChanged(SettingMenu *menu, SettingMenuItem *menu_item, int id)
{
    Browser_RequestRefreshPreview(1);
    return 0;
}

int Setting_onRewindEnabledOptionChanged(SettingMenu *menu, SettingMenuItem *menu_item, int id)
{
    if (Emu_IsGameLoaded())
    {
        if (misc_config.enable_rewind)
            Emu_InitRewind();
        else
            Emu_DeinitRewind();
    }
    return 0;
}

int Setting_onSaveScreenshotItemClick(SettingMenu *menu, SettingMenuItem *menu_item, int id)
{
    int ret = 0;
    char path[MAX_PATH_LENGTH];

    if (MakeScreenshotPath(path) < 0)
        goto FAILED;
    if (Emu_SaveVideoScreenshot(path) < 0)
        goto FAILED;

END:
    if (ret < 0)
        GUI_ShowToast(cur_lang[LANG_SAVE_SCREENSHOT_FAILED], 1);
    else
        GUI_ShowToast(cur_lang[LANG_SAVE_SCREENSHOT_OK], 1);
    return ret;

FAILED:
    ret = -1;
    goto END;
}

int Setting_onSavePreviewItemClick(SettingMenu *menu, SettingMenuItem *menu_item, int id)
{
    int ret = 0;
    char path[MAX_PATH_LENGTH];

    if (MakePreviewPath(path, "png") < 0)
        goto FAILED;
    if (Emu_SaveVideoScreenshot(path) < 0)
        goto FAILED;

END:
    if (ret < 0)
    {
        GUI_ShowToast(cur_lang[LANG_SAVE_PREVIEW_FAILED], 1);
    }
    else
    {
        Browser_RequestRefreshPreview(1);
        GUI_ShowToast(cur_lang[LANG_SAVE_PREVIEW_OK], 1);
    }
    return ret;

FAILED:
    ret = -1;
    goto END;
}

int Setting_onResetMiscConfigItemClick(SettingMenu *menu, SettingMenuItem *menu_item, int id)
{
    ResetMiscConfig();
    Setting_UpdateMenu(menu);
    menu->option_changed = 1;
    return 0;
}

//--------------------------------------------------------------------------------------------------------
//                          App menu callbacks
//--------------------------------------------------------------------------------------------------------
int Setting_onAppMenuFinish(SettingMenu *menu)
{
    if (menu->option_changed)
    {
        SaveAppConfig(setting_config_type);
        menu->option_changed = 0;
    }
    return 0;
}

int Setting_onPreviewOptionChanged(SettingMenu *menu, SettingMenuItem *menu_item, int id)
{
    Browser_RequestRefreshPreview(1);
    return 0;
}

int Setting_onLanguageOptionChanged(SettingMenu *menu, SettingMenuItem *menu_item, int id)
{
    StrArrayOption *option = (StrArrayOption *)menu_item->option_data;
    if (!option)
        return -1;

    app_config.language = GetLangIndexByConfigValue(*option->value);
    SetCurrentLang(app_config.language);
    Setting_UpdateMenu(menu);
    return 0;
}

int Setting_onResetAppConfigItemClick(SettingMenu *menu, SettingMenuItem *menu_item, int id)
{
    ResetAppConfig();
    setting_language_config_value = GetConfigValueByLangIndex(app_config.language);
    SetCurrentLang(app_config.language);
    Browser_RequestRefreshPreview(1);
    Setting_UpdateMenu(menu);
    menu->option_changed = 1;
    return 0;
}
