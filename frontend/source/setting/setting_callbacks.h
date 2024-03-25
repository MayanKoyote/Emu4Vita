#ifndef __M_SETTING_CALLBACKS_H__
#define __M_SETTING_CALLBACKS_H__

#include "setting_types.h"

//--------------------------------------------------------------------------------------------------------
//                          Main menu callbacks
//--------------------------------------------------------------------------------------------------------
int Setting_onResumeGameItemClick(SettingMenu *menu, SettingMenuItem *menu_item, int id);
int Setting_onResetGameItemClick(SettingMenu *menu, SettingMenuItem *menu_item, int id);
int Setting_onExitGameItemClick(SettingMenu *menu, SettingMenuItem *menu_item, int id);
int Setting_onDiskControlItemClick(SettingMenu *menu, SettingMenuItem *menu_item, int id);
int Setting_onExitToArchItemClick(SettingMenu *menu, SettingMenuItem *menu_item, int id);
int Setting_onExitAppItemClick(SettingMenu *menu, SettingMenuItem *menu_item, int id);

//--------------------------------------------------------------------------------------------------------
//                          State menu callbacks
//--------------------------------------------------------------------------------------------------------
int Setting_onStateMenuStart(SettingMenu *menu);
int Setting_onStateMenuFinish(SettingMenu *menu);
int Setting_onStateMenuDraw(SettingMenu *menu);
int Setting_onStateMenuCtrl(SettingMenu *menu);

//--------------------------------------------------------------------------------------------------------
//                          Graphics menu callbacks
//--------------------------------------------------------------------------------------------------------
int Setting_onGraphicsMenuFinish(SettingMenu *menu);
int Setting_onGraphicsMenuOptionChanged(SettingMenu *menu, SettingMenuItem *menu_item, int id);
int Setting_onResetGraphicsConfigItemClick(SettingMenu *menu, SettingMenuItem *menu_item, int id);

//--------------------------------------------------------------------------------------------------------
//                          Control menu callbacks
//--------------------------------------------------------------------------------------------------------
int Setting_onControlMenuFinish(SettingMenu *menu);
int Setting_onResetControlConfigItemClick(SettingMenu *menu, SettingMenuItem *menu_item, int id);

//--------------------------------------------------------------------------------------------------------
//                          Hotkey menu callbacks
//--------------------------------------------------------------------------------------------------------
int Setting_onHotkeyMenuFinish(SettingMenu *menu);
int Setting_onResetHotkeyConfigItemClick(SettingMenu *menu, SettingMenuItem *menu_item, int id);

//--------------------------------------------------------------------------------------------------------
//                          Core menu callbacks
//--------------------------------------------------------------------------------------------------------
int Setting_onCoreMenuFinish(SettingMenu *menu);
int Setting_onCoreMenuOptionChanged(SettingMenu *menu, SettingMenuItem *menu_item, int id);
int Setting_onResetCoreConfigItemClick(SettingMenu *menu, SettingMenuItem *menu_item, int id);

//--------------------------------------------------------------------------------------------------------
//                          Cheat menu callbacks
//--------------------------------------------------------------------------------------------------------
int Setting_onCheatMenuFinish(SettingMenu *menu);
int Setting_onResetCheatConfigItemClick(SettingMenu *menu, SettingMenuItem *menu_item, int id);

//--------------------------------------------------------------------------------------------------------
//                          Misc menu callbacks
//--------------------------------------------------------------------------------------------------------
int Setting_onMiscMenuFinish(SettingMenu *menu);
int Setting_onAutoSaveLoadOptionChanged(SettingMenu *menu, SettingMenuItem *menu_item, int id);
int Setting_onRewindEnabledOptionChanged(SettingMenu *menu, SettingMenuItem *menu_item, int id);
int Setting_onSaveScreenshotItemClick(SettingMenu *menu, SettingMenuItem *menu_item, int id);
int Setting_onSavePreviewItemClick(SettingMenu *menu, SettingMenuItem *menu_item, int id);
int Setting_onResetMiscConfigItemClick(SettingMenu *menu, SettingMenuItem *menu_item, int id);

//--------------------------------------------------------------------------------------------------------
//                          App menu callbacks
//--------------------------------------------------------------------------------------------------------
int Setting_onAppMenuFinish(SettingMenu *menu);
int Setting_onPreviewOptionChanged(SettingMenu *menu, SettingMenuItem *menu_item, int id);
int Setting_onLanguageOptionChanged(SettingMenu *menu, SettingMenuItem *menu_item, int id);
int Setting_onResetAppConfigItemClick(SettingMenu *menu, SettingMenuItem *menu_item, int id);

#endif
