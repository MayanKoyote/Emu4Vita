#ifndef __M_SETTING_OPTION_H__
#define __M_SETTING_OPTION_H__

#include "setting_types.h"

//--------------------------------------------------------------------------------------------------------
//                          String array option
//--------------------------------------------------------------------------------------------------------
int Setting_onStrArrayOptionItemClick(SettingMenu *menu, SettingMenuItem *menu_item, int id);
int Setting_onStrArrayOptionItemUpdate(SettingMenu *menu, SettingMenuItem *menu_item, int id);
int Setting_onStrArrayOptionClean(void *option_data);

//--------------------------------------------------------------------------------------------------------
//                          Integer array option
//--------------------------------------------------------------------------------------------------------
int Setting_onIntArrayOptionItemClick(SettingMenu *menu, SettingMenuItem *menu_item, int id);
int Setting_onIntArrayOptionItemUpdate(SettingMenu *menu, SettingMenuItem *menu_item, int id);
int Setting_onIntArrayOptionClean(void *option_data);

//--------------------------------------------------------------------------------------------------------
//                          Integer range option
//--------------------------------------------------------------------------------------------------------
int Setting_onIntRangeOptionItemClick(SettingMenu *menu, SettingMenuItem *menu_item, int id);
int Setting_onIntRangeOptionItemUpdate(SettingMenu *menu, SettingMenuItem *menu_item, int id);
int Setting_onIntRangeOptionClean(void *option_data);

//--------------------------------------------------------------------------------------------------------
//                          Key map option
//--------------------------------------------------------------------------------------------------------
int Setting_OnKeyMapOptionItemClick(SettingMenu *menu, SettingMenuItem *menu_item, int id);
int Setting_onKeyMapOptionItemUpdate(SettingMenu *menu, SettingMenuItem *menu_item, int id);
int Setting_onKeyMapOptionClean(void *option_data);

#endif