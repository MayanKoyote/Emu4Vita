#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <psp2/kernel/processmgr.h>

#include <vita2d.h>

#include "gui.h"
#include "file.h"
#include "config.h"
#include "utils.h"

typedef struct GUI_Font
{
    vita2d_pgf *font;
    int font_size;
    int line_space;
    int line_height;
} GUI_Font;

#define DEFAULT_FONT_SIZE 20
#define DEFAULT_LINE_SPACE 2

static GUI_Font *gui_font = NULL;

int GUI_InitFont()
{
    char path[MAX_PATH_LENGTH];

    if (gui_font)
        return 0;

    AppLog("[GUI] Font init...\n");

    gui_font = calloc(1, sizeof(GUI_Font));
    if (!gui_font)
        goto FAILED;

    if (private_assets_dir)
    {
        snprintf(path, MAX_PATH_LENGTH, "%s/%s", private_assets_dir, FONT_PGF_NAME);
        gui_font->font = vita2d_load_custom_pgf(path);
    }
    if (!gui_font->font && public_assets_dir)
    {
        snprintf(path, MAX_PATH_LENGTH, "%s/%s", public_assets_dir, FONT_PGF_NAME);
        gui_font->font = vita2d_load_custom_pgf(path);
    }

    if (!gui_font->font)
        goto FAILED;

    GUI_SetFontSize(DEFAULT_FONT_SIZE);
    GUI_SetLineSpace(DEFAULT_LINE_SPACE);

    AppLog("[GUI] Font init OK!\n");
    return 0;

FAILED:
    AppLog("[GUI] Font init failed!\n");
    sceKernelExitProcess(0);
    return -1;
}

void GUI_DeinitFont()
{
    if (gui_font)
    {
        AppLog("[GUI] Font deinit...\n");

        if (gui_font->font)
            vita2d_free_pgf(gui_font->font);
        free(gui_font);
        gui_font = NULL;

        AppLog("[GUI] Font deinit OK!\n");
    }
}

int GUI_GetFontSize()
{
    return gui_font->font_size;
}

void GUI_SetFontSize(int size)
{
    gui_font->font_size = size;
    gui_font->line_height = vita2d_pgf_get_lineheight(gui_font->font, size);
    AppLog("[GUI] font_size = %d, line_height = %d\n", gui_font->font_size, gui_font->line_height);
}

int GUI_GetLineHeight()
{
    return gui_font->line_height;
}

int GUI_GetLineSpace()
{
    return gui_font->line_space;
}

void GUI_SetLineSpace(int space)
{
    gui_font->line_space = space;
    vita2d_pgf_set_linespace(gui_font->font, space);
}

int GUI_DrawText(int x, int y, unsigned int color, const char *text)
{
    return vita2d_pgf_draw_text(gui_font->font, x, y, color, gui_font->font_size, text);
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
    return vita2d_pgf_text_width(gui_font->font, gui_font->font_size, text);
}

int GUI_GetTextHeight(const char *text)
{
    return vita2d_pgf_text_height(gui_font->font, gui_font->font_size, text);
}
