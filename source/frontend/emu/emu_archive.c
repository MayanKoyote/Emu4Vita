#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <psp2/kernel/processmgr.h>
#include <psp2/io/fcntl.h>

#include "archive/archive_7z.h"
#include "archive/archive_zip.h"
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

static ArchiveRomDriver *archiver_rom_drivers[] = {
    &archive_zip_driver,
    &archive_7z_driver,
};
#define N_ARCHIVER_ROM_DRIVERS (sizeof(archiver_rom_drivers) / sizeof(ArchiveRomDriver *))

static void makeCacheRomName(const char *archive_path, const char *entry_name, char *rom_name, size_t rom_name_size)
{
    MakeBaseName(rom_name, archive_path, rom_name_size);
    const char *ext = strrchr(entry_name, '.');
    if (ext)
        strcat(rom_name, ext);
}

int Archive_GetDriverIndex(const char *ext)
{
    int i;
    for (i = 0; i < N_ARCHIVER_ROM_DRIVERS; i++)
    {
        if (strcasecmp(ext, archiver_rom_drivers[i]->extension) == 0)
            return i;
    }

    return -1;
}

ArchiveRomDriver *Archive_GetDriver(int index)
{
    ArchiveRomDriver *driver = NULL;

    if (index >= 0 && index < N_ARCHIVER_ROM_DRIVERS)
    {
        driver = archiver_rom_drivers[index];
    }

    return driver;
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
    int type = GetFileType(path);
    if (type < n_core_valid_extensions)
        return 0;

    int n = 0;
    char entry_name[MAX_PATH_LENGTH];

    ArchiveRomDriver *driver = Archive_GetDriver(type - n_core_valid_extensions);
    if (!driver)
        return 0;

    if (driver->openRom(path, NULL, entry_name) <= 0)
        goto END;

    // 缓存文件名：例GBA: game1.zip ==> game1.gba（忽略压缩包内的rom文件名，以zip文件名为rom名）
    char rom_name[MAX_NAME_LENGTH];
    makeCacheRomName(path, entry_name, rom_name, sizeof(rom_name));

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
    driver->closeRom();

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
        if (res > 0)
        {
            // printf("StringGetLine: line = %s\n", line);
            if (StringReadConfigLine(line, &name, &value) >= 0)
            {
                // printf("StringReadConfigLine: name = %s, value = %s\n", name, value);
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

static int Archive_FindRomCache(int rom_crc, const char *rom_name)
{
    int i;
    for (i = 0; i < MAX_CACHE_SIZE; i++)
    {
        if (archive_cache_entries[i].exist && archive_cache_entries[i].crc == rom_crc && strcasecmp(archive_cache_entries[i].name, rom_name) == 0)
        {
            AppLog("[ARCHIVE] Archive_FindRomCache OK: index = %d, crc = %08X, name = %s\n", i, rom_crc, rom_name);
            return i;
        }
    }

    AppLog("[ARCHIVE] Archive_FindRomCache failed: %s\n", rom_name);
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

int Archive_GetRomMemory(const char *archive_path, void **buf, size_t *size, ArchiveRomDriver *driver)
{
    int ret = -1;
    uint32_t rom_crc;
    char entry_name[MAX_NAME_LENGTH];

    if (!driver)
        goto FAILED;

    ret = driver->openRom(archive_path, &rom_crc, entry_name);
    if (ret <= 0)
        goto FAILED;

#ifdef WANT_SAVE_MEM_ROM_CACHE
    // 缓存文件名：例GBA: game1.zip ==> game1.gba（忽略压缩包内的rom文件名，以zip文件名为rom名）
    char rom_name[MAX_NAME_LENGTH];
    char rom_path[MAX_PATH_LENGTH];
    makeCacheRomName(archive_path, entry_name, rom_name, sizeof(rom_name));
    sprintf(rom_path, "%s/%s", CORE_CACHE_DIR, rom_name);

    int index = Archive_FindRomCache(rom_crc, rom_name);
    if (index >= 0)
    {
        archive_cache_entries[index].ltime = sceKernelGetProcessTimeWide();
        ret = AllocateReadFileEx(rom_path, buf, size);
        goto END;
    }
#endif

    ret = driver->extractRomMemory(buf, size);
#ifdef WANT_SAVE_MEM_ROM_CACHE
    if (ret >= 0) // 保存memory rom cache
        Archive_SaveMemRomCache(rom_crc, rom_name, *buf, *size);
#endif

END:
    if (driver)
        driver->closeRom();

    if (ret < 0)
        AppLog("[ARCHIVE] Archive_GetRomMemory failed!\n");
    else
        AppLog("[ARCHIVE] Archive_GetRomMemory OK!\n");

    return ret;

FAILED:
    ret = -1;
    goto END;
}

int Archive_GetRomPath(const char *archive_path, char *rom_path, ArchiveRomDriver *driver)
{
    int ret = -1;
    uint32_t rom_crc;
    char entry_name[MAX_NAME_LENGTH];

    if (!driver)
        goto FAILED;

    ret = driver->openRom(archive_path, &rom_crc, entry_name);
    if (ret <= 0)
        goto FAILED;

    // 缓存文件名：例GBA: game1.zip ==> game1.gba（忽略压缩包内的rom文件名，以zip文件名为rom名）
    char rom_name[MAX_NAME_LENGTH];
    makeCacheRomName(archive_path, entry_name, rom_name, sizeof(rom_name));
    sprintf(rom_path, "%s/%s", CORE_CACHE_DIR, rom_name);
    CreateFolder(CORE_CACHE_DIR);

    int index = Archive_FindRomCache(rom_crc, rom_name);
    if (index < 0)
    {
        // 未找到cache，执行解压
        ret = driver->extractRom(rom_path);
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
    if (driver)
        driver->closeRom();

    if (ret < 0)
        AppLog("[ARCHIVE] Archive_GetRomPath failed!\n");
    else
        AppLog("[ARCHIVE] Archive_GetRomPath OK: %s\n", rom_path);

    return ret;

FAILED:
    ret = -1;
    goto END;
}

#ifdef WANT_SAVE_MEM_ROM_CACHE
typedef struct
{
    uint32_t crc;               // crc32
    char name[MAX_NAME_LENGTH]; // rom名称
    const void *buf;            // 缓存
    size_t size;                // 缓存大小
} MemRomInfo;

static SceUID archive_save_thread = -1;

static int saveMemRomCacheThreadFunc(SceSize args, void *argp)
{
    AppLog("[ARCHIVE] Archive_SaveMemRomCache thread start.\n");

    MemRomInfo *rom_info = (MemRomInfo *)argp;
    char rom_path[MAX_PATH_LENGTH];
    snprintf(rom_path, sizeof(rom_path), "%s/%s", CORE_CACHE_DIR, rom_info->name);
    // AppLog("[ARCHIVE] Archive_SaveMemRomCache: rom_path = %s, rom_size = %u\n", rom_path, rom_info->size);

    CreateFolder(CORE_CACHE_DIR);

    if (WriteFileEx(rom_path, rom_info->buf, rom_info->size) == rom_info->size)
    {
        Archive_AddCacheEntry(rom_info->crc, rom_info->name);
        AppLog("[ARCHIVE] Archive_SaveMemRomCache OK!\n");
    }
    else
    {
        sceIoRemove(rom_path);
        AppLog("[ARCHIVE] Archive_SaveMemRomCache failed!\n");
    }

    AppLog("[ARCHIVE] Archive_SaveMemRomCache thread exit.\n");
    sceKernelExitDeleteThread(0);
    return 0;
}

int Archive_SaveMemRomCache(uint32_t crc, const char *rom_name, const void *buf, size_t size)
{
    Archive_WaitThreadEnd();

    archive_save_thread = sceKernelCreateThread("save_rom_cache_thread", saveMemRomCacheThreadFunc, 0x10000100 + 20, 0x10000, 0, 0, NULL);
    if (archive_save_thread < 0)
        return -1;

    MemRomInfo rom_info;
    strcpy(rom_info.name, rom_name);
    rom_info.crc = crc;
    rom_info.size = size;
    rom_info.buf = buf;
    rom_info.size = size;

    return sceKernelStartThread(archive_save_thread, sizeof(rom_info), &rom_info);
}

// 防止未写完cache时，缓存就被其它函数清除
int Archive_WaitThreadEnd()
{
    if (archive_save_thread >= 0)
    {
        AppLog("[ARCHIVE] Wait Archive_SaveMemRomCache thread end...\n");
        sceKernelWaitThreadEnd(archive_save_thread, NULL, NULL);
        sceKernelDeleteThread(archive_save_thread);
        archive_save_thread = -1;
    }
    return 0;
}
#endif
