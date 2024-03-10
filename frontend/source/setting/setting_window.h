#ifndef __M_SETTING_WINDOW_H__
#define __M_SETTING_WINDOW_H__

#include "setting_types.h"

typedef struct SettingWindow SettingWindow;

SettingWindow *Setting_CreateWindow();
int Setting_DestroyWindow(SettingWindow *window);

int Setting_OpenWindow(SettingWindow *window);
int Setting_CloseWindow(SettingWindow *window);

int Setting_SetWindowAutoFree(SettingWindow *window, int auto_free);
int Setting_SetWindowContext(SettingWindow *window, SettingContext *context);

int Setting_GetWindowMenuLayoutPosition(int *layout_x, int *layout_y);
int Setting_GetWindowMenuAvailableSize(int *available_w, int *available_h);

#endif
