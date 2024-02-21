#ifndef __M_GUI_FONT_H__
#define __M_GUI_FONT_H__

int GUI_InitFont();
void GUI_DeinitFont();

int GUI_GetFontSize();
void GUI_SetFontSize(int size);
int GUI_GetLineHeight();
int GUI_GetLineSpace();
void GUI_SetLineSpace(int space);

int GUI_DrawText(int x, int y, unsigned int color, const char *text);
int GUI_DrawTextf(int x, int y, unsigned int color, const char *text, ...);
int GUI_GetTextWidth(const char *text);
int GUI_GetTextHeight(const char *text);

#endif