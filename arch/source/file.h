#ifndef __M_FILE_H__
#define __M_FILE_H__

#define MAX_PATH_LENGTH 1024
#define MAX_NAME_LENGTH 256
#define MAX_SHORT_NAME_LENGTH 64

int ReadFile(const char *file, void *buf, int size);
int WriteFile(const char *file, const void *buf, int size);
int AllocateReadFile(const char *file, void **buffer);

int CheckFileExist(const char *file);
int CheckFolderExist(const char *folder);

int CreateFolder(const char *path);

char *GetBaseDirectory(const char *path);
char *GetFilename(const char *path);
char *GetBaseFilename(const char *path);

int GetFileSize(const char *file);

#endif
