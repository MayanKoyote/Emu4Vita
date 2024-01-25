#ifndef __M_GUI_LIB_H__
#define __M_GUI_LIB_H__

int GUI_getFontSize();
void GUI_setFontSize(int size);
int GUI_getLineHeight();

int GUI_getLineSpace();
void GUI_setLineSpace(int space);

int GUI_drawText(int x, int y, unsigned int color, const char *text);
int GUI_drawTextf(int x, int y, unsigned int color, const char *text, ...);

int GUI_getTextWidth(const char *text);
int GUI_getTextHeight(const char *text);

void GUI_startDrawing();
void GUI_endDrawing();

int GUI_initLib();
int GUI_deinitLib();

void vita2d_draw_empty_rectangle(float x, float y, float w, float h, float line_size, unsigned int color);

#endif
