#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include <psp2/io/devctl.h>
#include <psp2/io/dirent.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <png.h>

#include "file.h"
#include "utils.h"

#define SCE_ERROR_ERRNO_EEXIST 0x80010011

int ReadFile(const char *file, void *buf, int size)
{
    SceUID fd = sceIoOpen(file, SCE_O_RDONLY, 0);
    if (fd < 0)
        return fd;

    int read = sceIoRead(fd, buf, size);

    sceIoClose(fd);
    return read;
}

int WriteFile(const char *file, const void *buf, int size)
{
    SceUID fd = sceIoOpen(file, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
    if (fd < 0)
        return fd;

    int written = sceIoWrite(fd, buf, size);

    sceIoClose(fd);
    return written;
}

int AllocateReadFile(const char *file, void **buffer)
{
    SceUID fd = sceIoOpen(file, SCE_O_RDONLY, 0);
    if (fd < 0)
        return fd;

    int size = sceIoLseek32(fd, 0, SCE_SEEK_END);
    sceIoLseek32(fd, 0, SCE_SEEK_SET);

    *buffer = malloc(size);
    if (!*buffer)
    {
        sceIoClose(fd);
        return -1;
    }

    int read = sceIoRead(fd, *buffer, size);
    sceIoClose(fd);

    return read;
}

int GetFileSize(const char *file)
{
    SceUID fd = sceIoOpen(file, SCE_O_RDONLY, 0);
    if (fd < 0)
        return fd;

    int fileSize = sceIoLseek(fd, 0, SCE_SEEK_END);

    sceIoClose(fd);
    return fileSize;
}

int CheckFileExist(const char *file)
{
    SceIoStat st;
    if (sceIoGetstat(file, &st) >= 0)
        return SCE_S_ISREG(st.st_mode);
    return 0;
}

int CheckFolderExist(const char *folder)
{
    SceIoStat st;
    if (sceIoGetstat(folder, &st) >= 0)
        return SCE_S_ISDIR(st.st_mode);
    return 0;
}

int CreateFolder(const char *path)
{
    int ret = 0;
    char buf[MAX_PATH_LENGTH];

    if (strlen(path) > MAX_PATH_LENGTH)
        return -1;

    strcpy(buf, path);
    AddEndSlash(buf);

    char ch;
    int i;
    for (i = 0; i < MAX_PATH_LENGTH; i++)
    {
        if (buf[i] == '/')
        {
            ch = buf[i];
            buf[i] = '\0';
            ret = sceIoMkdir(buf, 0777);
            buf[i] = ch;
        }
    }

    if (ret == SCE_ERROR_ERRNO_EEXIST)
        ret = 0;
    return ret;
}

char *GetBaseDirectory(const char *path)
{
    int len = strlen(path);
    if (len <= 0)
        return NULL;

    if (path[len - 1] == '/')
        len--;

    int sep_start = 0;
    int sep_end = len;

    int i;
    for (i = sep_end - 1; i >= 0; i--)
    {
        if (path[i] == '/')
        {
            sep_end = i;
            break;
        }
    }

    if (sep_end == len)
        return NULL;

    int new_len = sep_end - sep_start;
    if (new_len <= 0)
        return NULL;

    char *res = (char *)malloc(new_len + 1);
    if (!res)
        return NULL;

    strncpy(res, path + sep_start, new_len);
    res[new_len] = '\0';

    return res;
}

char *GetFilename(const char *path)
{
    int len = strlen(path);
    if (len <= 0)
        return NULL;

    if (path[len - 1] == '/' || path[len - 1] == ':')
        return NULL; // no file

    int sep_start = 0;
    int sep_end = len;

    int i;
    for (i = sep_end - 1; i >= 0; i--)
    {
        if (path[i] == '/' || path[i] == ':')
        {
            sep_start = i + 1;
            break;
        }
    }

    if (sep_start == 0)
        return NULL;

    int new_len = sep_end - sep_start;
    if (new_len <= 0)
        return NULL;

    char *res = (char *)malloc(new_len + 1);
    if (!res)
        return NULL;

    strncpy(res, path + sep_start, new_len);
    res[new_len] = '\0';

    return res;
}

char *GetBaseFilename(const char *path)
{
    int len = strlen(path);
    if (len <= 0)
        return NULL;

    int sep_start = 0;
    int sep_end = len;

    int i;
    for (i = sep_end - 1; i >= 0; i--)
    {
        if (path[i] == '/' || path[i] == ':')
        {
            sep_start = i + 1;
            break;
        }
        else if ((path[i] == '.') && (sep_end == len))
        {
            sep_end = i;
        }
    }

    int new_len = sep_end - sep_start;
    if (new_len <= 0)
        return NULL;

    char *res = (char *)malloc(new_len + 1);
    if (!res)
        return NULL;

    strncpy(res, path + sep_start, new_len);
    res[new_len] = '\0';

    return res;
}
