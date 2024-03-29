#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <psp2/io/fcntl.h>

#include "activity/activity.h"
#include "emu/emu.h"
#include "file.h"
#include "utils.h"
#include "config.h"

static int loadMemoryFile(int id)
{
    size_t dst_size = retro_get_memory_size(id);
    if (dst_size == 0)
        return -1;

    void *dst_data = retro_get_memory_data(id);
    if (dst_data == NULL)
        return -1;

    char path[MAX_PATH_LENGTH];
    MakeSavefilePath(path, id);
    SceUID fd = sceIoOpen(path, SCE_O_RDONLY, 0);
    if (fd < 0)
        return fd;

    int64_t src_size = sceIoLseek(fd, 0, SCE_SEEK_END);
    if (src_size <= 0)
    {
        APP_LOG("[SAVE] Load memory file: get file size failed\n");
        sceIoClose(fd);
        return -1;
    }
    if (src_size > dst_size)
    {
        APP_LOG("[SAVE] Load memory file: SRAM is larger than implementation expects\n");
        sceIoClose(fd);
        return -1;
    }

    void *src_data = malloc(src_size);
    if (src_data == NULL)
    {
        sceIoClose(fd);
        APP_LOG("[SAVE] Load memory file: alloc buf failed\n");
        return -1;
    }

    sceIoLseek(fd, 0, SCE_SEEK_SET);
    char *buf = (char *)src_data;
    int64_t remaining = src_size;
    int64_t transfer = TRANSFER_SIZE;

    while (remaining > 0)
    {
        if (remaining < TRANSFER_SIZE)
            transfer = remaining;
        else
            transfer = TRANSFER_SIZE;

        int read = sceIoRead(fd, buf, transfer);
        if (read < 0)
        {
            free(src_data);
            sceIoClose(fd);
            APP_LOG("[SAVE] Load memory file: read file failed\n");
            return -1;
        }
        if (read == 0)
            break;

        buf += read;
        remaining -= read;
    }
    sceIoClose(fd);

    memcpy(dst_data, src_data, src_size);
    free(src_data);

    return 0;
}

static int saveMemoryFile(int id)
{
    size_t size = retro_get_memory_size(id);
    if (size == 0)
        return -1;

    void *data = retro_get_memory_data(id);
    if (data == NULL)
        return -1;

    char path[MAX_PATH_LENGTH];
    char parent_dir[MAX_PATH_LENGTH];

    MakeSavefilePath(path, id);
    MakeParentDir(parent_dir, path, MAX_PATH_LENGTH);
    CreateFolder(parent_dir);

    if (WriteFileEx(path, data, size) != size)
    {
        APP_LOG("[SAVE] Save memory file failed: %s!\n", path);
        sceIoRemove(path);
        return -1;
    }

    return 0;
}

int Emu_LoadSrm()
{
    loadMemoryFile(RETRO_MEMORY_SAVE_RAM);
    loadMemoryFile(RETRO_MEMORY_RTC);
    return 0;
}

int Emu_SaveSrm()
{
    saveMemoryFile(RETRO_MEMORY_SAVE_RAM);
    saveMemoryFile(RETRO_MEMORY_RTC);
    return 0;
}

int Emu_DeleteSrm()
{
    char path[MAX_PATH_LENGTH];
    MakeSavefilePath(path, RETRO_MEMORY_SAVE_RAM);
    sceIoRemove(path);
    MakeSavefilePath(path, RETRO_MEMORY_RTC);
    sceIoRemove(path);
    return 0;
}