#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <psp2/kernel/processmgr.h>
#include <psp2/io/fcntl.h>

#include "archive/zip_archive.h"
#include "archive/7z_archive.h"
#include "emu/emu.h"
#include "file.h"
#include "config.h"
#include "utils.h"
#include "utils_string.h"

typedef struct
{
    int exist;                  // 缓存存在
    uint32_t crc;               // crc32
    uint64_t ltime;             // 加载时间
    char name[MAX_NAME_LENGTH]; // rom名称
} ArchiveEntry;

#define MAX_CACHE_SIZE 5
#define ARCHIVE_CACHE_CONFIG_PATH CORE_CACHE_DIR "/archive_cache.txt"

static ArchiveEntry archive_cache_entries[MAX_CACHE_SIZE];

int Archive_GetMode(const char *path)
{
    int mode = ARCHIVE_MODE_NO;

    if (core_want_ext_zip_mode || core_want_ext_7z_mode)
    {
        const char *ext = strrchr(path, '.');
        if (ext++)
        {
            if (core_want_ext_zip_mode && strcasecmp(ext, "zip") == 0)
                mode = ARCHIVE_MODE_ZIP;

            if (mode == ARCHIVE_MODE_NO && core_want_ext_7z_mode && strcasecmp(ext, "7z") == 0)
                mode = ARCHIVE_MODE_7Z;
        }
    }

    return mode;
}

int Archive_CleanCache(int index)
{
    if (archive_cache_entries[index].exist)
    {
        AppLog("[ARCHIVE] Archive_CleanCache: index = %d, crc = %08X, name = %s\n", index, archive_cache_entries[index].crc, archive_cache_entries[index].name);

        char path[MAX_PATH_LENGTH];
        snprintf(path, sizeof(path), "%s/%s", CORE_CACHE_DIR, archive_cache_entries[index].name);
        sceIoRemove(path);
        memset(&archive_cache_entries[index], 0, sizeof(archive_cache_entries[index]));
        return 1;
    }

    return 0;
}

int Archive_CleanCacheByPath(const char *path)
{
    int archive_mode = Archive_GetMode(path);
    if (archive_mode == ARCHIVE_MODE_NO)
        return 0;

    int n = 0;
    int ret = -1;
    char entry_name[MAX_PATH_LENGTH];

    if (archive_mode == ARCHIVE_MODE_ZIP)
        ret = ZIP_OpenRom(path, NULL, entry_name);
    else if (archive_mode == ARCHIVE_MODE_7Z)
        ret = SevenZ_OpenRom(path, NULL, entry_name);

    if (ret <= 0)
        goto END;

    // 缓存文件名：例GBA: game1.zip ==> game1.gba（忽略压缩包内的rom文件名，以zip文件名为rom名）
    char rom_name[MAX_NAME_LENGTH];
    MakeBaseName(rom_name, path, sizeof(rom_name));
    const char *ext = strrchr(entry_name, '.');
    if (ext)
        strcat(rom_name, ext);

    int i;
    for (i = 0; i < MAX_CACHE_SIZE; i++)
    {
        if (archive_cache_entries[i].exist && strcasecmp(archive_cache_entries[i].name, rom_name) == 0)
        {
            Archive_CleanCache(i);
            n = 1;
            break;
        }
    }

END:
    if (archive_mode == ARCHIVE_MODE_ZIP)
        ZIP_CloseRom();
    else if (archive_mode == ARCHIVE_MODE_7Z)
        SevenZ_CloseRom();
    return n;
}

int Archive_CleanAllCaches()
{
    int n = 0;

    int i;
    for (i = 0; i < MAX_CACHE_SIZE; i++)
    {
        if (archive_cache_entries[i].exist)
        {
            Archive_CleanCache(i);
            n++;
        }
    }

    return n;
}

int Archive_LoadCacheConfig()
{
    memset(archive_cache_entries, 0, sizeof(archive_cache_entries));

    void *buffer = NULL;
    size_t size = 0;
    if (AllocateReadFile(ARCHIVE_CACHE_CONFIG_PATH, &buffer, &size) < 0)
        return -1;

    int res = 0;
    char *p = (char *)buffer;

    // Skip UTF-8 bom
    uint32_t bom = 0xBFBBEF;
    if (memcmp(p, &bom, 3) == 0)
    {
        p += 3;
        size -= 3;
    }

    int index = 0;
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
                    archive_cache_entries[index].exist = 1;
                    archive_cache_entries[index].crc = StringToHexdecimal(name);
                    archive_cache_entries[index].ltime = 0; // 加载时间初始化为0
                    strcpy(archive_cache_entries[index].name, value);
                    index++;
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
    } while (res > 0 && index < MAX_CACHE_SIZE);

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
    for (i = 0; i < MAX_CACHE_SIZE; i++)
    {
        if (archive_cache_entries[i].exist)
        {
            snprintf(string, sizeof(string), "%08X=\"%s\"\n", archive_cache_entries[i].crc, archive_cache_entries[i].name);
            if ((ret = sceIoWrite(fd, string, strlen(string))) < 0)
                break;
        }
    }

    sceIoClose(fd);
    if (ret < 0)
        sceIoRemove(ARCHIVE_CACHE_CONFIG_PATH);
    return ret;
}

static int Archive_FindRomCache(int rom_crc, const char *rom_name, char *rom_path)
{
    int i;
    for (i = 0; i < MAX_CACHE_SIZE; i++)
    {
        if (archive_cache_entries[i].exist && archive_cache_entries[i].crc == rom_crc && strcasecmp(archive_cache_entries[i].name, rom_name) == 0)
        {
            sprintf(rom_path, "%s/%s", CORE_CACHE_DIR, rom_name);
            AppLog("[ARCHIVE] Archive_FindRomCache OK: index = %d, crc = %08X, name = %s\n", i, rom_crc, rom_name);
            return i;
        }
    }

    AppLog("[ARCHIVE] FindRomCache failed: %s\n", rom_name);
    return -1;
}

static int getInsertCacheEntriesIndex()
{
    int index = 0;
    uint64_t ltime = archive_cache_entries[index].ltime;

    int i;
    for (i = 0; i < MAX_CACHE_SIZE; i++)
    {
        // 如果此处为空，返回当前index
        if (!archive_cache_entries[i].exist)
            return i;

        // 获取最小加载时间的缓存条目
        if (archive_cache_entries[i].ltime < ltime)
            index = i;
    }

    return index;
}

static int Archive_AddCacheEntry(int crc, const char *rom_name)
{
    int index = getInsertCacheEntriesIndex(); // 获取新条目插入位置

    if (archive_cache_entries[index].exist)
        Archive_CleanCache(index);

    archive_cache_entries[index].exist = 1;
    // 设置新条目的crc
    archive_cache_entries[index].crc = crc;
    // 设置新条目的name
    strcpy(archive_cache_entries[index].name, rom_name);
    // 设置新条目的ltime为当前线程时间
    archive_cache_entries[index].ltime = sceKernelGetProcessTimeWide();

    Archive_SaveCacheConfig();

    AppLog("[ARCHIVE] Archive_AddCacheEntry OK: index = %d, crc = %08X, name = %s\n", index, crc, rom_name);
    return index;
}

int Archive_GetRomMemory(const char *archive_path, void **buf, size_t *size, int archive_mode)
{
    int ret = -1;

    if (archive_mode == ARCHIVE_MODE_ZIP)
    {
        if (ZIP_OpenRom(archive_path, NULL, NULL) <= 0)
            goto FAILED;
        ret = ZIP_ExtractRomMemory(buf, size);
    }
    else if (archive_mode == ARCHIVE_MODE_7Z)
    {
        if (SevenZ_OpenRom(archive_path, NULL, NULL) <= 0)
            goto FAILED;
        ret = SevenZ_ExtractRomMemory(buf, size);
    }

END:
    if (archive_mode == ARCHIVE_MODE_ZIP)
        ZIP_CloseRom();
    else if (archive_mode == ARCHIVE_MODE_7Z)
        SevenZ_CloseRom();
    if (ret < 0)
        AppLog("[ARCHIVE] Archive_GetRomMemory failed!\n");
    else
        AppLog("[ARCHIVE] Archive_GetRomMemory OK!\n");
    return ret;

FAILED:
    ret = -1;
    goto END;
}

int Archive_GetRomPath(const char *archive_path, char *rom_path, int archive_mode)
{
    int ret = -1;
    uint32_t rom_crc;
    char entry_name[MAX_PATH_LENGTH];

    if (archive_mode == ARCHIVE_MODE_ZIP)
        ret = ZIP_OpenRom(archive_path, &rom_crc, entry_name);
    else if (archive_mode == ARCHIVE_MODE_7Z)
        ret = SevenZ_OpenRom(archive_path, &rom_crc, entry_name);

    if (ret <= 0)
        goto FAILED;

    // 缓存文件名：例GBA: game1.zip ==> game1.gba（忽略压缩包内的rom文件名，以zip文件名为rom名）
    char rom_name[MAX_NAME_LENGTH];
    MakeBaseName(rom_name, archive_path, sizeof(rom_name));
    const char *ext = strrchr(entry_name, '.');
    if (ext)
        strcat(rom_name, ext);

    int index = Archive_FindRomCache(rom_crc, rom_name, rom_path);
    if (index < 0)
    {
        ret = -1;
        // 未找到cache，执行解压
        if (archive_mode == ARCHIVE_MODE_ZIP)
            ret = ZIP_ExtractRom(rom_name, rom_path);
        else if (archive_mode == ARCHIVE_MODE_7Z)
            ret = SevenZ_ExtractRom(rom_name, rom_path);

        if (ret < 0)
            goto FAILED;

        // 添加新的cache条目
        Archive_AddCacheEntry(rom_crc, rom_name);
    }
    else
    {
        // 更新ltime为当前线程时间
        archive_cache_entries[index].ltime = sceKernelGetProcessTimeWide();
    }

END:
    if (archive_mode == ARCHIVE_MODE_ZIP)
        ZIP_CloseRom();
    else if (archive_mode == ARCHIVE_MODE_7Z)
        SevenZ_CloseRom();
    if (ret < 0)
        AppLog("[ARCHIVE] Archive_GetRomPath failed!\n");
    else
        AppLog("[ARCHIVE] Archive_GetRomPath OK: %s\n", rom_path);
    return ret;

FAILED:
    ret = -1;
    goto END;
}

/*
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
}*/