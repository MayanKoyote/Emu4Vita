#ifndef __M_GUI_H__
#define __M_GUI_H__

#include "graphics.h"

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

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 544
#define SCREEN_HALF_WIDTH (SCREEN_WIDTH / 2)
#define SCREEN_HALF_HEIGHT (SCREEN_HEIGHT / 2)

#define DEFAULT_FONT_SCALE 1.0f
#define DEFALUT_FONT_COLOR WHITE

#define DEFALUT_BG_COLOR COLOR_SET_ALPHA(0x1F1F1F, 0xAF)

#define SCROLL_BAR_WIDTH 6

void GUI_init();
void GUI_deinit();
void GUI_main();

extern char STR_BUTTON_ENTER[4], STR_BUTTON_CANCEL[4];

extern float MAIN_FREE_DRAW_WIDTH, MAIN_FREE_DRAW_HEIGHT;
extern float MAIN_FREE_DRAW_SX, MAIN_FREE_DRAW_DX, MAIN_FREE_DRAW_SY, MAIN_FREE_DRAW_DY;

#endif
