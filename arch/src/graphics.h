#ifndef __M_GRAPHICS_H__
#define __M_GRAPHICS_H__

int GUI_initFonts();
void GUI_deinitFonts();

int GUI_drawText(int x, int y, unsigned int color, const char *text);
int GUI_drawTextf(int x, int y, unsigned int color, const char *text, ...);

float GUI_getLineHeight();

int GUI_getTextWidth(const char *text);
int GUI_getTextHeight(const char *text);

void GUI_startDrawing();
void GUI_endDrawing();

void vita2d_draw_empty_rectangle(float x, float y, float w, float h, float l, unsigned int color);

#endif
