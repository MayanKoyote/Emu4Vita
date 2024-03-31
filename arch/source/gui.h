#ifndef __M_GUI_H__
#define __M_GUI_H__

#include "gui_lib.h"

enum Colors
{
    // Primary colors
    RED = 0xFF0000FF,
    GREEN = 0xFF00FF00,
    BLUE = 0xFFFF0000,
    // Secondary colors
    CYAN = 0xFFFFFF00,
    MAGENTA = 0xFFFF00FF,
    YELLOW = 0xFF00FFFF,
    // Tertiary colors
    AZURE = 0xFFFF7F00,
    VIOLET = 0xFFFF007F,
    ROSE = 0xFF7F00FF,
    ORANGE = 0xFF007FFF,
    CHARTREUSE = 0xFF00FF7F,
    SPRING_GREEN = 0xFF7FFF00,
    // Grayscale
    WHITE = 0xFFFFFFFF,
    LITEGRAY = 0xFFBFBFBF,
    GRAY = 0xFF7F7F7F,
    DARKGRAY = 0xFF3F3F3F,
    BLACK = 0xFF000000
};

#define NOALPHA 0xFF
#define COLOR_GET_ALPHA(color) ((color >> 24) & 0xFF)
#define COLOR_SET_ALPHA(color, alpha) ((color & 0x00FFFFFF) | ((alpha & 0xFF) << 24))

#define GUI_SCREEN_WIDTH 960
#define GUI_SCREEN_HEIGHT 544

void GUI_SetWallpaperTexture(vita2d_texture *texture);

int GUI_Init();
void GUI_Deinit();
void GUI_RunMain();

int GUI_LockDrawMutex();
int GUI_UnlockDrawMutex();

int GUI_GetActivityLayoutPosition(int *x, int *y);
int GUI_GetActivityAvailableSize(int *w, int *h);

#endif
