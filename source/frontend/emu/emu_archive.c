#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <psp2/io/fcntl.h>
#include <psp2/kernel/threadmgr.h>
#include "emu/emu.h"
#include "config.h"
#include "utils.h"
#include "7z.h"

#define ARCHIVE_CACHE_CONFIG_PATH CORE_CACHE_DIR "/cache.txt"

ArchiveCacheEntry archive_cache_entries[MAX_CACHE_SIZE];
int archive_cache_num = 0;

int Archive_LoadCacheConfig()
{
    memset(archive_cache_entries, 0, sizeof(archive_cache_entries));
    archive_cache_num = 0;

    void *buffer = NULL;
    int size = AllocateReadFile(ARCHIVE_CACHE_CONFIG_PATH, &buffer);
    if (size < 0)
        return size;

    int res = 0;
    char *p = (char *)buffer;

    // Skip UTF-8 bom
    uint32_t bom = 0xBFBBEF;
    if (memcmp(p, &bom, 3) == 0)
    {
        p += 3;
        size -= 3;
    }

    char path[MAX_PATH_LENGTH];
    char *line = NULL;
    char *name = NULL, *value = NULL;

    do
    {
        res = StringGetLine(p, size, &line);
        // printf("StringGetLine: line = %s\n", line);
        if (res > 0)
        {
            if (StringReadConfigLine(line, &name, &value) >= 0)
            {
                // printf("StringGetLine: name = %s, value = %s\n", name, value);
                sprintf(path, "%s/%s", CORE_CACHE_DIR, value);
                if (CheckFileExist(path))
                {
                    archive_cache_entries[archive_cache_num].crc = StringToHexdecimal(name);
                    archive_cache_entries[archive_cache_num].ltime = 0; // 加载时间初始化为0
                    strcpy(archive_cache_entries[archive_cache_num].name, value);
                    archive_cache_num++;
                }
            }

            if (line)
                free(line);
            line = NULL;
            if (name)
                free(name);
            name = NULL;
            if (value)
                free(value);
            value = NULL;

            size -= res;
            p += res;
        }
    } while (res > 0 && archive_cache_num < MAX_CACHE_SIZE);

    free(buffer);

    return 0;
}

int Archive_SaveCacheConfig()
{
    SceUID fd = sceIoOpen(ARCHIVE_CACHE_CONFIG_PATH, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
    if (fd < 0)
        return fd;

    int ret = 0;
    char string[MAX_CONFIG_LINE_LENGTH];
    int i;
    for (i = 0; i < archive_cache_num; i++)
    {
        snprintf(string, sizeof(string), "%08X=\"%s\"\n", archive_cache_entries[i].crc, archive_cache_entries[i].name);
        if ((ret = sceIoWrite(fd, string, strlen(string))) < 0)
            break;
    }

    sceIoClose(fd);
    if (ret < 0)
        sceIoRemove(ARCHIVE_CACHE_CONFIG_PATH);
    return ret;
}

int Archive_FindRomCache(uint32_t crc, const char *rom_name, char *rom_path)
{
    int i;
    for (i = 0; i < archive_cache_num; i++)
    {
        if (archive_cache_entries[i].crc == crc && strcasecmp(archive_cache_entries[i].name, rom_name) == 0)
        {
            sprintf(rom_path, "%s/%s", CORE_CACHE_DIR, rom_name);
            AppLog("[ARCHIVE] FindRomCache OK: %d, %s\n", i, rom_name);
            return i;
        }
    }

    AppLog("[ARCHIVE] FindRomCache failed: %s\n", rom_name);
    return -1;
}

static int Archive_GetInsertCacheEntriesIndex()
{
    if (archive_cache_num < MAX_CACHE_SIZE)
        return archive_cache_num;

    int index = 0;
    uint64_t ltime = archive_cache_entries[0].ltime;

    int i;
    for (i = 1; i < MAX_CACHE_SIZE; i++)
    {
        // 获取最小加载时间的缓存条目
        if (archive_cache_entries[i].ltime < ltime)
            index = i;
    }

    return index;
}

int Archive_InsertRomCache(uint32_t crc, const char *rom_name)
{
    int index = Archive_GetInsertCacheEntriesIndex(); // 获取新条目插入位置

    if (archive_cache_num >= MAX_CACHE_SIZE) // 缓存条目已达到最大数
    {
        // 删除要替换的旧条目指向的rom文件
        char tmp_path[MAX_PATH_LENGTH];
        sprintf(tmp_path, "%s/%s", CORE_CACHE_DIR, archive_cache_entries[index].name);
        sceIoRemove(tmp_path);
    }
    else
    {
        archive_cache_num++;
    }

    memset(&archive_cache_entries[index], 0, sizeof(archive_cache_entries[index]));
    archive_cache_entries[index].crc = crc;
    strcpy(archive_cache_entries[index].name, rom_name);

    Archive_SaveCacheConfig();

    return index;
}

int Archive_GetRomMemory(const char *archive_path, void **buf, size_t *size, int mode)
{
    switch (mode)
    {
    case ZIP_MODE:
        return ZIP_GetRomMemory(archive_path, buf, size);
    case SEVENZ_MODE:
        return SevenZ_GetRomMemory(archive_path, buf, size);
    default:
        return -1;
    }
}

int Archive_GetRomPath(const char *archive_path, char *rom_path, int mode)
{
    switch (mode)
    {
    case ZIP_MODE:
        return ZIP_GetRomPath(archive_path, rom_path);
    case SEVENZ_MODE:
        return SevenZ_GetRomPath(archive_path, rom_path);
    default:
        return -1;
    }
}

typedef struct
{
    char name[256];
    uint32_t crc;
    char *buf;
    size_t size;
} RomCacheThreadArgs;

int WriteRomCahceThread(SceSize args, void *argp)
{
    RomCacheThreadArgs *arg = (RomCacheThreadArgs *)argp;
    char rom_path[MAX_NAME_LENGTH];
    strcpy(rom_path, CORE_CACHE_DIR "/");
    strcat(rom_path, arg->name);
    if (WriteFile(rom_path, arg->buf, arg->size) > 0)
    {
        Archive_InsertRomCache(arg->crc, arg->name);
    }

    free(arg->buf);
    AppLog("[ARCHIVE] AsyncWriteRomCache thread end\n");

    sceKernelExitDeleteThread(0);
    return 0;
}

void Archive_AsyncWriteRomCache(uint32_t crc, const char *rom_name, const char *buf, size_t size)
{
    AppLog("[ARCHIVE] AsyncWriteRomCache start\n");
    int write_thread = sceKernelCreateThread("write_rom_cache_thread", WriteRomCahceThread, 0x10000100 + 20, 0x10000, 0, 0, NULL);
    if (write_thread >= 0)
    {
        RomCacheThreadArgs args;
        strcpy(args.name, rom_name);
        args.crc = crc;
        args.size = size;
        args.buf = malloc(size);
        if (args.buf)
        {
            memcpy(args.buf, buf, size);
            sceKernelStartThread(write_thread, sizeof(args), &args);
        }
    }
}