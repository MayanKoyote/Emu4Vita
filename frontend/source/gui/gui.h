#ifndef __M_GUI_H__
#define __M_GUI_H__

typedef enum GUI_ImageId
{
    ID_GUI_IMAGE_WALLPAPER,
    ID_GUI_IMAGE_SPLASH,
    ID_GUI_IMAGE_CHECKBOX_ON,
    ID_GUI_IMAGE_CHECKBOX_OFF,
    ID_GUI_IMAGE_RADIOBUTTON_ON,
    ID_GUI_IMAGE_RADIOBUTTON_OFF,
    N_GUI_IMAGES,
} GUI_ImageId;

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

#include "gui_lib.h"
#include "gui_color.h"
#include "gui_font.h"
#include "gui_ctrl.h"
#include "gui_shader.h"
#include "gui_init.h"

void GUI_DrawVerticalScrollbar(int track_x, int track_y, int track_height, int max_len, int draw_len, int scroll_len, int draw_track);

void GUI_SetImage(GUI_ImageId id, GUI_Texture *texture);
GUI_Texture *GUI_GetImage(GUI_ImageId id);

int GUI_InitDraw();
int GUI_DeinitDraw();

int GUI_LockDrawMutex();
int GUI_UnlockDrawMutex();

void GUI_RunMain();

#include "layout/Layout.h"
#include "gui_activity.h"
#include "gui_window.h"
#include "gui_toast.h"
#include "window/window.h"

#endif