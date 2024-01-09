#include <unordered_map>
#include <string>

extern "C"
{
#include <stdio.h>
#include <stdlib.h>
#include <psp2/io/dirent.h>
#include <psp2/io/stat.h>
#include <psp2/io/fcntl.h>
#include <psp2/kernel/threadmgr.h>
#include "config.h"
#include "file.h"
#include "utils.h"
#include "zip_cache.h"
#include "zip/zip.h"
}

#define CACHE_NUM 5
#define CHECK_SIZE 0x20
#define ZIP_CACHE_CONFIG_PATH CORE_ZIPCACHE_DIR "/cache.txt"
#define LINE_BUF_SIZE 256 + 16

struct CachedItem
{
    time_t time;
    std::string name;
};

struct ExtractArgs
{
    char name[256];
    uint32_t crc32;
};

class ZipCache : public std::unordered_map<uint32_t, CachedItem>
{
public:
    // Config 文件格式：
    // 每行： 【8位16进制crc32】=【rom文件完整路径】
    // 例:    ABCD1234=ux0:data/EMU4VITA/【core】/zipcache/xxxx.gba
    void Load();      // 从 ZIP_CACHE_CONFIG_PATH 读取 cache 信息
    void Save();      // 把 cache 信息，写入 ZIP_CACHE_CONFIG_PATH
    void CheckSize(); // 如超过 CACHE_NUM，则删除最老的 cache rom
};

ZipCache zip_cache;

void ZipCache::Load()
{
    this->clear();

    char line[LINE_BUF_SIZE];
    FILE *fp = fopen(ZIP_CACHE_CONFIG_PATH, "r");
    if (!fp)
        return;

    while (fgets(line, LINE_BUF_SIZE, fp))
    {
        char *key = strtok(line, "=\r\n");
        uint32_t crc32 = strtoul(key, NULL, 16);
        if (crc32 == LONG_MAX)
            continue;

        char *name = strtok(NULL, "=\r\n");
        SceIoStat stat = {0};
        if (sceIoGetstat(name, &stat) < 0)
            continue;

        time_t time;
        sceRtcGetTime_t(&stat.st_mtime, &time);
        (*this)[crc32] = {time, name};
        AppLog("[ZIP] Load zip cache: %08x = \"%s\"\n", crc32, name);
    }

    fclose(fp);
};

void ZipCache::Save()
{
    FILE *fp = fopen(ZIP_CACHE_CONFIG_PATH, "w");
    if (!fp)
        return;

    for (const auto &iter : *this)
    {
        fprintf(fp, "%08X=%s\n", iter.first, iter.second.name.c_str());
    }

    fclose(fp);
};

void ZipCache::CheckSize()
{
    if (this->size() <= CACHE_NUM)
    {
        return;
    }

    uint32_t earliest_crc32 = this->begin()->first;
    time_t earliest_time = this->begin()->second.time;
    for (const auto &iter : *this)
    {
        if (iter.second.time < earliest_time)
        {
            earliest_crc32 = iter.first;
            earliest_time = iter.second.time;
        }
    }

    sceIoRemove((*this)[earliest_crc32].name.c_str());
    this->erase(earliest_crc32);
}

void InitZipCache()
{
    SceUID dfd = sceIoDopen(CORE_ZIPCACHE_DIR);
    if (dfd < 0)
    {
        CreateFolder(CORE_ZIPCACHE_DIR);
        return;
    }
    else
    {
        zip_cache.Load();
        zip_cache.CheckSize();
    }
}

uint32_t GetZipRomCrc32(const char *name)
{
    uint32_t crc32 = 0;
    struct zip_t *zip = zip_open(name, 0, 'r');
    if (zip)
    {
        size_t num = zip_entries_total(zip);
        for (size_t i = 0; i < num && crc32 == 0; i++)
        {
            if (zip_entry_openbyindex(zip, i) == 0)
            {
                const char *entry_name = zip_entry_name(zip);
                if (IsValidFile(entry_name))
                {
                    crc32 = zip_entry_crc32(zip);
                }
                zip_entry_close(zip);
            }
        }
        zip_close(zip);
    }

    return crc32;
}

bool ExtractByCrc32(const char *name, uint32_t crc32)
{
    struct zip_t *zip = zip_open(name, 0, 'r');
    if (!zip)
        return false;

    bool extracted = false;
    size_t num = zip_entries_total(zip);
    for (size_t i = 0; i < num && extracted == false; i++)
    {
        if (zip_entry_openbyindex(zip, i) != 0)
            continue;

        if (zip_entry_crc32(zip) == crc32)
        {
            const char *entry_name = zip_entry_name(zip);
            const char *ext = strrchr(entry_name, '.');

            char cache_name[256];
            strcpy(cache_name, CORE_ZIPCACHE_DIR "/");

            const char *pure_path = strchr(name, ':');
            pure_path = pure_path ? pure_path + 1 : name;
            const char *pure_zip_name = strrchr(pure_path, '/');
            pure_zip_name = pure_zip_name ? pure_zip_name + 1 : pure_path;
            strcat(cache_name, pure_zip_name);
            *strrchr(cache_name, '.') = '\x00';
            strcat(cache_name, ext);

            if (zip_entry_fread(zip, cache_name) == 0)
            {
                extracted = true;

                SceDateTime sce_time;
                time_t time;
                sceRtcGetCurrentClockLocalTime(&sce_time);
                sceRtcGetTime_t(&sce_time, &time);
                zip_cache[crc32] = {time, cache_name};

                zip_cache.CheckSize();
                zip_cache.Save();
                AppLog("[ZIP] Extract: %08x = \"%s\"\n", crc32, cache_name);
            }
        }
        zip_entry_close(zip);
    }
    zip_close(zip);
    return extracted;
}

int ExtractThread(SceSize args, void *argp)
{
    ExtractArgs *arg = (ExtractArgs *)argp;
    ExtractByCrc32(arg->name, arg->crc32);
    sceKernelExitDeleteThread(0);
    return 0;
}

const char *GetZipCacheRomPath(const char *name)
{
    uint32_t crc32 = GetZipRomCrc32(name);

    auto iter = zip_cache.find(crc32);
    if (iter != zip_cache.end())
    {
        AppLog("[ZIP] Hit zip cache: %08x %s\n", crc32, iter->second.name.c_str());
        return iter->second.name.c_str();
    }

    return ExtractByCrc32(name, crc32) ? zip_cache[crc32].name.c_str() : NULL;
}

int64_t GetZipCacheRomMemory(const char *name, void **rom)
{
    *rom = NULL;
    int64_t size = 0;
    uint32_t crc32 = GetZipRomCrc32(name);

    auto iter = zip_cache.find(crc32);
    if (iter != zip_cache.end())
    {
        const char *cache_name = iter->second.name.c_str();
        AppLog("[ZIP] Hit zip cache: %08x %s\n", crc32, cache_name);
        size = GetFileSize(cache_name);
        if (size > 0)
        {
            *rom = (char *)malloc(size);
            if (!*rom)
                return -1;
            ReadFile(cache_name, *rom, size);
        }
        return size;
    }

    struct zip_t *zip = zip_open(name, 0, 'r');
    if (!zip)
        return false;

    size_t num = zip_entries_total(zip);
    for (size_t i = 0; i < num && *rom == NULL; i++)
    {
        if (zip_entry_openbyindex(zip, i) != 0)
            continue;

        if (zip_entry_crc32(zip) == crc32)
        {
            size = zip_entry_size(zip);
            *rom = (char *)malloc(size);
            if (!*rom)
                return -1;

            if (zip_entry_noallocread(zip, *rom, size) != size)
                return -1;
        }
        zip_entry_close(zip);
    }
    zip_close(zip);

    int extract_thread = sceKernelCreateThread("extract_thread", ExtractThread, 0x10000100, 0x10000, 0, 0, NULL);
    if (extract_thread >= 0)
    {
        ExtractArgs args;
        strcpy(args.name, name);
        args.crc32 = crc32;
        sceKernelStartThread(extract_thread, sizeof(args), &args);
    }

    return size;
}