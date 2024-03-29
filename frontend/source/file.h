#ifndef __M_FILE_H__
#define __M_FILE_H__

#include <stdint.h>

#define MAX_PATH_LENGTH 1024
#define MAX_NAME_LENGTH 256
#define MAX_SHORT_NAME_LENGTH 64

#define DIRECTORY_SIZE (4 * 1024)
#define TRANSFER_SIZE (128 * 1024)

extern char **core_valid_extensions;
extern int n_core_valid_extensions;

int IsValidFile(const char *path);
int GetRomType(const char *filename);

int MakeParentDirEx(char *parent, int parent_size, const char *path, int path_len);
int MakeFileNameEx(char *name, int name_size, const char *path, int path_len);
int MakeBaseNameEx(char *name, int name_size, const char *path, int path_len);
int MakeParentDir(char *parent_dir, const char *path, int size);
int MakeFileName(char *name, const char *path, int size);
int MakeBaseName(char *name, const char *path, int size);

int ReadFile(const char *file, void *buffer, size_t size);
int WriteFile(const char *file, const void *buffer, size_t size);
int AllocateReadFile(const char *file, void **buffer, size_t *buffersize);
int64_t ReadFileEx(const char *file, const void *buffer, size_t size);
int64_t WriteFileEx(const char *file, const void *buffer, size_t size);
int64_t AllocateReadFileEx(const char *file, void **buffer, size_t *buffersize);

int CheckFileExist(const char *file);
int CheckFolderExist(const char *folder);

int CreateFolder(const char *path);

int GetFileSize(const char *file);

int RemovePath(const char *path);
int CopyFile(const char *src_path, const char *dst_path);
int CopyPath(const char *src_path, const char *dst_path);
int MovePath(const char *src_path, const char *dst_path);

int WritePngFile(const char *path, unsigned char *pixels, int width, int height, int bit_depth);

#endif
