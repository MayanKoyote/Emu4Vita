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

int GUI_InitFont()
{
    gui_font = vita2d_load_custom_pgf(FONT_PGF_PATH);
    if (!gui_font)
        gui_font = vita2d_load_default_pgf();
    if (!gui_font)
        return -1;

    GUI_SetFontSize(DEFAULT_FONT_SIZE);
    GUI_SetLineSpace(DEFAULT_LINE_SPACE);

    return 0;
}

void GUI_DeinitFont()
{
    if (gui_font)
    {
        vita2d_free_pgf(gui_font);
        gui_font = NULL;
    }
}

int GUI_SetFontSize(int size)
{
    gui_font_size = size;
    gui_line_height = vita2d_pgf_get_lineheight(gui_font, size);
    return 0;
}

int GUI_SetLineSpace(int space)
{
    gui_line_space = space;
    vita2d_pgf_set_linespace(gui_font, space);
    return 0;
}

int GUI_GetFontSize()
{
    return gui_font_size;
}

int GUI_GetLineSpace()
{
    return gui_line_space;
}

int GUI_GetLineHeight()
{
    return gui_line_height;
}

int GUI_DrawText(int x, int y, unsigned int color, const char *text)
{
    return vita2d_pgf_draw_text(gui_font, x, y, color, gui_font_size, text);
}

int GUI_DrawTextf(int x, int y, unsigned int color, const char *text, ...)
{
    char buf[1024];
    va_list argptr;
    va_start(argptr, text);
    vsnprintf(buf, sizeof(buf), text, argptr);
    va_end(argptr);

    return GUI_DrawText(x, y, color, buf);
}

int GUI_GetTextWidth(const char *text)
{
    return vita2d_pgf_text_width(gui_font, gui_font_size, text);
}

int GUI_GetTextHeight(const char *text)
{
    return vita2d_pgf_text_height(gui_font, gui_font_size, text);
}

void vita2d_draw_empty_rectangle(float x, float y, float w, float h, float size, unsigned int color)
{
    int x2 = x + w;
    int y2 = y + h;
    vita2d_draw_rectangle(x, y, size, h - size, color);
    vita2d_draw_rectangle(x + size, y, w - size, size, color);
    vita2d_draw_rectangle(x2 - size, y + size, size, h - size, color);
    vita2d_draw_rectangle(x, y2 - size, w - size, size, color);
}

void GUI_StartDrawing()
{
    vita2d_start_drawing();
    vita2d_clear_screen();
}

void GUI_EndDrawing()
{
    vita2d_end_drawing();
    // vita2d_common_dialog_update();
    vita2d_swap_buffers();
}

int GUI_InitLib()
{
    vita2d_init();
    vita2d_set_vblank_wait(1);
    GUI_InitFont();
    return 0;
}

int GUI_DeinitLib()
{
    vita2d_fini();
    GUI_DeinitFont();
    return 0;
}
