#ifndef __M_GUI_LIB_H__
#define __M_GUI_LIB_H__

#include <vita2d.h>

int GUI_SetFontSize(int size);
int GUI_SetLineSpace(int space);

int GUI_GetFontSize();
int GUI_GetLineSpace();
int GUI_GetLineHeight();

int GUI_DrawText(int x, int y, unsigned int color, const char *text);
int GUI_DrawTextf(int x, int y, unsigned int color, const char *text, ...);

int GUI_GetTextWidth(const char *text);
int GUI_GetTextHeight(const char *text);

void vita2d_draw_empty_rectangle(float x, float y, float w, float h, float size, unsigned int color);

void GUI_StartDrawing();
void GUI_EndDrawing();

int GUI_InitLib();
int GUI_DeinitLib();

#endif
