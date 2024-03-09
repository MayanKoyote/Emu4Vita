#ifndef __M_SPLASH_ACITVITY_H__
#define __M_SPLASH_ACITVITY_H__

#include "gui/gui.h"

extern GUI_Activity splash_activity;

void Splash_AddLog(const char *text);
void Splash_AddLogf(const char *text, ...);
void Splash_SetListviewAutoScroll(int enable);
void Splash_SetBgTexture(GUI_Texture *texture);
void Splash_SetLogEnabled(int enabled);

#endif