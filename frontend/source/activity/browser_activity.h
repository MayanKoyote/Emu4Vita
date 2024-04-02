#ifndef __M_BROWSER_ACITVITY_H__
#define __M_BROWSER_ACITVITY_H__

#include "gui/gui.h"

extern GUI_Activity browser_activity;

int Browser_CurrentPathIsGame();
int Browser_MakeCurrentFileName(char *name);
int Browser_MakeCurrentFilePath(char *path);

GUI_Texture *GetDefaultPreviewTexture();

void Browser_RequestRefreshPreview(int urgent);
int Browser_ChangeDirByFilePath(const char *path);
int Browser_ChangeDirBySaveFile(const char *path);

#endif
