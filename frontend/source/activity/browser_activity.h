#ifndef __M_BROWSER_ACITVITY_H__
#define __M_BROWSER_ACITVITY_H__

#include "gui/gui.h"

extern GUI_Activity browser_activity;

int CurrentPathIsGame();
int GetCurrentFileType();
int MakeCurrentFileName(char *name);
int MakeCurrentFilePath(char *path);
int MakePreviewPath(char *path, char *ext);
int MakeScreenshotPath(char *path);

GUI_Texture *GetDefaultPreviewTexture();

void Browser_RequestRefreshPreview(int urgent);

int Browser_ChangeDirByFilePath(const char *path);
int Browser_ChangeDirBySaveFile(const char *path);

#endif
