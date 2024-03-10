#ifndef __M_SETTING_H__
#define __M_SETTING_H__

#include "list/linked_list.h"
#include "setting_state.h"
#include "setting_overlay.h"

int Setting_Init();
int Setting_Deinit();

int Setting_SetCoreMenu(LinkedList *list);
int Setting_SetCheatMenu(LinkedList *list);
int Setting_SetOverlayOption(LinkedList *list);

int Setting_OpenMenu();
int Setting_CloseMenu();

#endif