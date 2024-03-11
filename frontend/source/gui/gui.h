#ifndef __M_GUI_H__
#define __M_GUI_H__

#include "gui_lib.h"
#include "gui_color.h"
#include "gui_font.h"
#include "gui_ctrl.h"
#include "gui_shader.h"
#include "gui_init.h"
#include "layout/Layout.h"

// Screen
#define GUI_SCREEN_WIDTH 960
#define GUI_SCREEN_HEIGHT 544
#define GUI_SCREEN_HALF_WIDTH (GUI_SCREEN_WIDTH / 2)
#define GUI_SCREEN_HALF_HEIGHT (GUI_SCREEN_HEIGHT / 2)

// Layout
#define GUI_DEF_MAIN_LAYOUT_PADDING 10

// Scroll bar
#define GUI_DEF_SCROLLBAR_SIZE 6
#define GUI_DEF_SCROLLBAR_COLOR_TRACK COLOR_ALPHA(COLOR_DARKGRAY, 0xAF)
#define GUI_DEF_SCROLLBAR_COLOR_THUMB COLOR_ALPHA(COLOR_LITEGRAY, 0x8F)

// Color
#define GUI_DEF_COLOR_TEXT COLOR_WHITE
#define GUI_DEF_COLOR_BG 0xAF0F0F0F
#define GUI_DEF_COLOR_FOCUS COLOR_ALPHA(COLOR_AZURE, 0xDF)

void GUI_DrawVerticalScrollbar(int track_x, int track_y, int track_height, int max_len, int draw_len, int scroll_len, int draw_track);

void GUI_SetBackToMainActivityEnabled(int enabled);
void GUI_SetDefaultWallpaper(GUI_Texture *texture);
void GUI_SetDefaultSplash(GUI_Texture *texture);
void GUI_SetCheckBoxTexture(GUI_Texture *on_texture, GUI_Texture *off_texture);
void GUI_SetRadioButtonTexture(GUI_Texture *on_texture, GUI_Texture *off_texture);

GUI_Texture *GUI_GetDefaultWallpaper();
GUI_Texture *GUI_GetDefaultSplash();
void GUI_GetCheckBoxTexture(GUI_Texture **on_texture, GUI_Texture **off_texture);
void GUI_GetRadioButtonTexture(GUI_Texture **on_texture, GUI_Texture **off_texture);

void GUI_Run();
int GUI_StartThreadRun(); // 有些时候我们的事件无法在线程里执行，所以可以将gui切换到线程里执行
void GUI_ExitThreadRun();
int GUI_IsInThreadRun();

#include "gui_activity.h"
#include "gui_window.h"
#include "gui_toast.h"
#include "window/window.h"

#endif