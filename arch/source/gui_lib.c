#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <vita2d.h>

#include "gui_lib.h"
#include "file.h"
#include "config.h"
#include "utils.h"

#define DEFAULT_FONT_SIZE 20
#define DEFAULT_LINE_SPACE 2

static vita2d_pgf *gui_font = NULL;
static int gui_font_size = DEFAULT_FONT_SIZE;
static int gui_line_height = DEFAULT_FONT_SIZE;
static int gui_line_space = DEFAULT_LINE_SPACE;

static int initFont()
{
    gui_font = vita2d_load_custom_pgf(FONT_PGF_PATH);
    if (!gui_font)
        gui_font = vita2d_load_default_pgf();
    if (!gui_font)
        return -1;

    GUI_setFontSize(DEFAULT_FONT_SIZE);
    GUI_setLineSpace(DEFAULT_LINE_SPACE);

    return 0;
}

static void deinitFont()
{
    if (gui_font)
    {
        vita2d_free_pgf(gui_font);
        gui_font = NULL;
    }
}

int GUI_getFontSize()
{
    return gui_font_size;
}

void GUI_setFontSize(int size)
{
    gui_font_size = size;
    gui_line_height = vita2d_pgf_get_lineheight(gui_font, size);
}

int GUI_getLineHeight()
{
    return gui_line_height;
}

int GUI_getLineSpace()
{
    return gui_line_space;
}

void GUI_setLineSpace(int space)
{
    gui_line_space = space;
    vita2d_pgf_set_linespace(gui_font, space);
}

int GUI_drawText(int x, int y, unsigned int color, const char *text)
{
    return vita2d_pgf_draw_text(gui_font, x, y, color, gui_font_size, text);
}

int GUI_drawTextf(int x, int y, unsigned int color, const char *text, ...)
{
    char buf[1024];
    va_list argptr;
    va_start(argptr, text);
    vsnprintf(buf, sizeof(buf), text, argptr);
    va_end(argptr);

    return GUI_drawText(x, y, color, buf);
}

int GUI_getTextWidth(const char *text)
{
    return vita2d_pgf_text_width(gui_font, gui_font_size, text);
}

int GUI_getTextHeight(const char *text)
{
    return vita2d_pgf_text_height(gui_font, gui_font_size, text);
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
}

int GUI_initLib()
{
    vita2d_init();
    vita2d_set_vblank_wait(1);
    initFont();
    return 0;
}

int GUI_deinitLib()
{
    vita2d_fini();
    deinitFont();
    return 0;
}

void vita2d_draw_empty_rectangle(float x, float y, float w, float h, float line_size, unsigned int color)
{
    float sx = x;
    float sy = y;
    float dx = x + w;
    float dy = y + h;
    vita2d_draw_rectangle(sx, sy, line_size, h - line_size, color);
    vita2d_draw_rectangle(sx + line_size, sy, w - line_size, line_size, color);
    vita2d_draw_rectangle(dx - line_size, sy + line_size, line_size, h - line_size, color);
    vita2d_draw_rectangle(sx, dy - line_size, w - line_size, line_size, color);
}
