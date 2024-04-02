#ifndef __M_PATH_H__
#define __M_PATH_H__

int CurrentPathIsGame();

int MakeCurrentGameName(char *name);
int MakeCurrentGamePath(char *path);

int MakeSavestateDir(char *path);
int MakeSavestatePath(char *path, int num);
int MakeSavefileDir(char *path);
int MakeSavefilePath(char *path, int id);
int MakeCheatPath(char *path);
int MakeCheatPath2(char *path);

int MakePreviewPath(char *path, char *ext);
int MakeScreenshotPath(char *path);
int MakeSplashPath(char *path);

#endif
