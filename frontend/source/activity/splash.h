#ifndef __M_SPLASH_H__
#define __M_SPLASH_H__

#include "gui/gui.h"

void Splash_AddLog(const char *text);
void Splash_AddLogf(const char *text, ...);
void Splash_SetAutoScrollListview(int enable);
void Splash_SetBgTexture(GUI_Texture *texture);
void Splash_SetLogEnabled(int enabled);

#endif