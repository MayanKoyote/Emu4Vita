#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <psp2/display.h>

#include <vita2d.h>

#include "gui.h"
#include "config.h"

#define DEFAULT_FONT_HEIGHT 17.402f
#define DEFAULT_LINE_HEIGHT 21.0f

static vita2d_pgf *font = NULL;
static float font_height = DEFAULT_FONT_HEIGHT;
static float font_scale = DEFAULT_FONT_SCALE;

int GUI_initFonts()
{
    font = vita2d_load_custom_pgf(FONT_PGF_PATH);
    if (!font)
    {
        font = vita2d_load_default_pgf();
        if (!font)
            return -1;
    }

    font_height = DEFAULT_FONT_HEIGHT;
    font_scale = DEFAULT_FONT_SCALE;

    return 0;
}

void GUI_deinitFonts()
{
    if (font)
        vita2d_free_pgf(font);
    font = NULL;
}

int GUI_drawText(int x, int y, unsigned int color, const char *text)
{
    return vita2d_pgf_draw_text(font, x, y + font_height, color, font_scale, text);
}

int GUI_drawTextf(int x, int y, unsigned int color, const char *text, ...)
{
    char buf[1024];
    va_list argptr;
    va_start(argptr, text);
    vsnprintf(buf, sizeof(buf), text, argptr);
    va_end(argptr);

    return vita2d_pgf_draw_text(font, x, y + font_height, color, font_scale, buf);
}

float GUI_getFontScale()
{
    return font_scale;
}

void GUI_setFontScale(float scale)
{
    font_scale = scale;
    font_height = DEFAULT_FONT_HEIGHT * font_scale;
}

float GUI_getLineHeight()
{
    return DEFAULT_LINE_HEIGHT * font_scale;
}

int GUI_getTextWidth(const char *text)
{
    return vita2d_pgf_text_width(font, font_scale, text);
}

int GUI_getTextHeight(const char *text)
{
    return vita2d_pgf_text_height(font, font_scale, text);
}

void GUI_startDrawing()
{
    vita2d_start_drawing();
    vita2d_clear_screen();
}

void GUI_endDrawing()
{
    vita2d_end_drawing();
    // vita2d_common_dialog_update();
    vita2d_swap_buffers();
    sceDisplayWaitVblankStart();
}

void vita2d_draw_empty_rectangle(float x, float y, float w, float h, float l, unsigned int color)
{
    float sx = x;
    float sy = y;
    float dx = x + w;
    float dy = y + h;
    vita2d_draw_rectangle(sx, sy, l, h - l, color);
    vita2d_draw_rectangle(sx + l, sy, w - l, l, color);
    vita2d_draw_rectangle(dx - l, sy + l, l, h - l, color);
    vita2d_draw_rectangle(sx, dy - l, w - l, l, color);
}
