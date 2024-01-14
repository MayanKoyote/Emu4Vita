#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <psp2/kernel/processmgr.h>
#include <psp2/io/fcntl.h>

#include "zip/zip.h"
#include "emu/emu.h"
#include "file.h"
#include "config.h"
#include "utils.h"
#include "utils_string.h"

typedef struct
{
    uint32_t crc;               // crc32
    uint64_t ltime;             // 加载时间
    char name[MAX_NAME_LENGTH]; // rom名称
} ZipCacheEntry;

#define MAX_CACHE_SIZE 5
#define ZIP_CACHE_CONFIG_PATH CORE_CACHE_DIR "/zip_cache.txt"

static ZipCacheEntry zip_cache_entries[MAX_CACHE_SIZE];
static int zip_cache_num = 0;
static struct zip_t *current_zip = NULL;

int ZIP_LoadCacheConfig()
{
    memset(zip_cache_entries, 0, sizeof(zip_cache_entries));
    zip_cache_num = 0;

    void *buffer = NULL;
    int size = AllocateReadFile(ZIP_CACHE_CONFIG_PATH, &buffer);
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
                    zip_cache_entries[zip_cache_num].crc = StringToHexdecimal(name);
                    zip_cache_entries[zip_cache_num].ltime = 0; // 加载时间初始化为0
                    strcpy(zip_cache_entries[zip_cache_num].name, value);
                    zip_cache_num++;
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
    } while (res > 0 && zip_cache_num < MAX_CACHE_SIZE);

    free(buffer);

    return 0;
}

int ZIP_SaveCacheConfig()
{
    SceUID fd = sceIoOpen(ZIP_CACHE_CONFIG_PATH, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
    if (fd < 0)
        return fd;

    int ret = 0;
    char string[MAX_CONFIG_LINE_LENGTH];
    int i;
    for (i = 0; i < zip_cache_num; i++)
    {
        snprintf(string, sizeof(string), "%08X=\"%s\"\n", zip_cache_entries[i].crc, zip_cache_entries[i].name);
        if ((ret = sceIoWrite(fd, string, strlen(string))) < 0)
            break;
    }

    sceIoClose(fd);
    if (ret < 0)
        sceIoRemove(ZIP_CACHE_CONFIG_PATH);
    return ret;
}

static int ZIP_FindRomCache(const char *rom_name, char *rom_path)
{
    if (!current_zip)
        return -1;

    uint32_t crc = zip_entry_crc32(current_zip);

    int i;
    for (i = 0; i < zip_cache_num; i++)
    {
        if (zip_cache_entries[i].crc == crc && strcasecmp(zip_cache_entries[i].name, rom_name) == 0)
        {
            sprintf(rom_path, "%s/%s", CORE_CACHE_DIR, rom_name);
            AppLog("[ZIP] FindRomCache OK: %d, %s\n", i, rom_name);
            return i;
        }
    }

    AppLog("[ZIP] FindRomCache failed: %s\n", rom_name);
    return -1;
}

static int getInsertCacheEntriesIndex()
{
    if (zip_cache_num < MAX_CACHE_SIZE)
        return zip_cache_num;

    int index = 0;
    uint64_t ltime = zip_cache_entries[0].ltime;

    int i;
    for (i = 1; i < MAX_CACHE_SIZE; i++)
    {
        // 对比最小加载时间
        if (zip_cache_entries[i].ltime < ltime)
            index = i;
    }

    return index;
}

static int ZIP_ExtractRomCache(const char *rom_name, char *rom_path)
{
    if (!current_zip)
        return -1;

    CreateFolder(CORE_CACHE_DIR);
    sprintf(rom_path, "%s/%s", CORE_CACHE_DIR, rom_name);

    if (zip_entry_fread(current_zip, rom_path) != 0)
    {
        RemovePath(rom_path);
        AppLog("[ZIP] ExtractRomCache failed: %s\n", rom_name);
        return -1;
    }

    int index = getInsertCacheEntriesIndex(); // 获取新条目插入位置

    if (zip_cache_num >= MAX_CACHE_SIZE) // 缓存条目已达到最大数
    {
        // 删除要插入的条目指向的rom文件
        char tmp_path[MAX_PATH_LENGTH];
        sprintf(tmp_path, "%s/%s", CORE_CACHE_DIR, zip_cache_entries[index].name);
        RemovePath(tmp_path);
    }
    else
    {
        zip_cache_num++;
    }

    memset(&zip_cache_entries[index], 0, sizeof(zip_cache_entries[index]));

    zip_cache_entries[index].crc = zip_entry_crc32(current_zip);
    strcpy(zip_cache_entries[index].name, rom_name);

    ZIP_SaveCacheConfig();

    AppLog("[ZIP] ExtractRomCache OK: %d, %s\n", index, rom_name);
    return index;
}

static int ZIP_GetRomEntry(const char *zip_path)
{
    if (current_zip)
        zip_close(current_zip);

    current_zip = zip_open(zip_path, 0, 'r');
    if (!current_zip)
        return -1;

    size_t total = zip_entries_total(current_zip);
    size_t i;
    for (i = 0; i < total; i++)
    {
        if (zip_entry_openbyindex(current_zip, i) == 0)
        {
            const char *entry_name = zip_entry_name(current_zip);
            if (IsValidFile(entry_name))
            {
                AppLog("[ZIP] ZIP_GetRomEntry OK!\n");
                return 1;
            }
            zip_entry_close(current_zip);
        }
    }

    if (current_zip)
    {
        zip_close(current_zip);
        current_zip = NULL;
    }
    AppLog("[ZIP] ZIP_GetRomEntry failed!\n");
    return 0;
}

int ZIP_GetRomMemory(const char *zip_path, void **buf, size_t *size)
{
    if (ZIP_GetRomEntry(zip_path) <= 0)
        return -1;

    if (zip_entry_read(current_zip, buf, size) <= 0)
    {
        if (current_zip)
        {
            zip_close(current_zip);
            current_zip = NULL;
        }
        AppLog("[ZIP] ZIP_GetRomMemory failed!\n");
        return -1;
    }

    AppLog("[ZIP] ZIP_GetRomMemory OK!\n");
    return 0;
}

int ZIP_GetRomPath(const char *zip_path, char *rom_path)
{
    if (ZIP_GetRomEntry(zip_path) <= 0)
        return -1;

    const char *entry_name = zip_entry_name(current_zip);
    const char *ext = strrchr(entry_name, '.');

    char rom_name[MAX_NAME_LENGTH];
    MakeBaseName(rom_name, zip_path, sizeof(rom_name));
    strcat(rom_name, ext);
    // printf("ZIP_GetRomPath: rom_name: %s\n", rom_name);

    int index = ZIP_FindRomCache(rom_name, rom_path);
    if (index < 0)
        index = ZIP_ExtractRomCache(rom_name, rom_path);
    if (index < 0)
    {
        if (current_zip)
        {
            zip_close(current_zip);
            current_zip = NULL;
        }
        AppLog("[ZIP] GetRomPath failed!\n");
        return -1;
    }

    // 设置rom加载时间为当前线程时间
    zip_cache_entries[index].ltime = sceKernelGetProcessTimeWide();
    AppLog("[ZIP] GetRomPath OK: %s\n", rom_path);
    return 0;
}
