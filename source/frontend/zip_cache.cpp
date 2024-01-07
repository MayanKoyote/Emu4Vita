#include <unordered_map>
#include <string>

extern "C"
{
#include <stdio.h>
#include <stdlib.h>
#include <psp2/io/dirent.h>
#include <psp2/io/stat.h>
#include <psp2/io/fcntl.h>
#include "config.h"
#include "file.h"
#include "utils.h"
#include "zip_cache.h"
#include "zip/zip.h"
}

#define CACHE_NUM 5
#define CHECK_SIZE 0x20

struct CachedItem
{
    time_t time;
    std::string name;
};

std::unordered_map<uint32_t, CachedItem> zip_cache;

void CheckZipCacheSize()
{
    if (zip_cache.size() <= CACHE_NUM)
    {
        return;
    }

    uint32_t earliest_crc32 = zip_cache.begin()->first;
    time_t earliest_time = zip_cache.begin()->second.time;
    for (const auto &iter : zip_cache)
    {
        if (iter.second.time < earliest_time)
        {
            earliest_crc32 = iter.first;
        }
    }

    sceIoRemove(zip_cache[earliest_crc32].name.c_str());
    zip_cache.erase(earliest_crc32);
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
        RefreshZipCache();
        CheckZipCacheSize();
    }
}

void RefreshZipCache()
{
    SceUID dfd = sceIoDopen(CORE_ZIPCACHE_DIR);
    if (dfd < 0)
        return;

    int res = 0;
    do
    {
        SceIoDirent dir = {0};
        res = sceIoDread(dfd, &dir);
        if (res > 0 && (!SCE_S_ISDIR(dir.d_stat.st_mode)) && IsValidFile(dir.d_name))
        {
            char *end;
            int crc32 = strtol(dir.d_name, &end, 16);
            if (crc32 != 0 && *end == '.')
            {
                time_t time;
                sceRtcGetTime_t(&dir.d_stat.st_mtime, &time);
                zip_cache[crc32] = {
                    time,
                    std::string(CORE_ZIPCACHE_DIR) + "/" + dir.d_name,
                };
            }
        }
    } while (res > 0);

    sceIoDclose(dfd);
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

const char *GetZipCacheRom(const char *name)
{
    uint32_t crc32 = GetZipRomCrc32(name);
    auto iter = zip_cache.find(crc32);
    if (iter != zip_cache.end())
        return iter->second.name.c_str();

    struct zip_t *zip = zip_open(name, 0, 'r');
    if (!zip)
        return NULL;

    bool extracted = false;
    size_t num = zip_entries_total(zip);
    for (size_t i = 0; i < num && extracted == false; i++)
    {
        if (zip_entry_openbyindex(zip, i) != 0)
            continue;

        if (zip_entry_crc32(zip) == crc32)
        {
            void *buf;
            int size = zip_entry_read(zip, &buf, NULL);
            if (size > 0)
            {
                const char *entry_name = zip_entry_name(zip);
                const char *ext = strrchr(entry_name, '.');

                char crc32_name[sizeof(CORE_ZIPCACHE_DIR) + 0x20];
                sprintf(crc32_name, "%s/%08X%s", CORE_ZIPCACHE_DIR, crc32, ext);
                WriteFile(crc32_name, buf, size);
                free(buf);
                extracted = true;

                SceDateTime sce_time;
                time_t time;
                sceRtcGetCurrentClockLocalTime(&sce_time);
                sceRtcGetTime_t(&sce_time, &time);
                zip_cache[crc32] = {time, crc32_name};

                CheckZipCacheSize();
            }
        }
        zip_entry_close(zip);
    }
    zip_close(zip);

    return extracted ? zip_cache[crc32].name.c_str() : NULL;
}
