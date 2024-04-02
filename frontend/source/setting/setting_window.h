#ifndef __M_SETTING_WINDOW_H__
#define __M_SETTING_WINDOW_H__

#include "setting_types.h"

typedef struct SettingWindow SettingWindow;

int SettingWindow_UpdateLayout(SettingWindow *window);

SettingWindow *SettingWindow_Create();
void SettingWindow_Destroy(SettingWindow *st_window);

int SettingWindow_Open(SettingWindow *st_window);
int SettingWindow_Close(SettingWindow *st_window);

int SettingWindow_SetAutoFree(SettingWindow *st_window, int auto_free);
int SettingWindow_SetContext(SettingWindow *st_window, SettingContext *context);

int SettingWindow_GetMenuLayoutPosition(int *layout_x, int *layout_y);
int SettingWindow_GetMenuAvailableSize(int *available_w, int *available_h);

#endif
