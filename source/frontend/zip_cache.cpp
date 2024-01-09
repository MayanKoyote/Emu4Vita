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
#define ZIP_CACHE_CONFIG_PATH CORE_ZIPCACHE_DIR "/cache.txt"
#define LINE_BUF_SIZE 256 + 16

struct CachedItem
{
    time_t time;
    std::string name;
};

class ZipCache : public std::unordered_map<uint32_t, CachedItem>
{
public:
    // Config 文件格式：
    // 每行： 【8位16进制crc32】=【rom文件完整路径】
    // 例:    ABCD1234=ux0:data/EMU4VITA/【core】/zipcache/xxxx.gba
    void Load();
    void Save();
    void CheckZipCacheSize();
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
        AppLog("Load zip cache: %08x = \"%s\"\n", crc32, name);
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

void ZipCache::CheckZipCacheSize()
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
        zip_cache.CheckZipCacheSize();
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

const char *GetZipCacheRom(const char *name)
{
    uint32_t crc32 = GetZipRomCrc32(name);
    auto iter = zip_cache.find(crc32);
    if (iter != zip_cache.end())
    {
        AppLog("Hit zip cache: %08x %s\n", crc32, iter->second.name.c_str());
        return iter->second.name.c_str();
    }
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

                char cache_name[256];
                strcpy(cache_name, CORE_ZIPCACHE_DIR "/");

                const char *pure_zip_name = strrchr(name + 4, '/'); // +4 for skip "ux0:"
                pure_zip_name = pure_zip_name ? pure_zip_name + 1 : name + 4;
                strcat(cache_name, pure_zip_name);
                *strrchr(cache_name, '.') = '\x00';
                strcat(cache_name, ext);

                WriteFile(cache_name, buf, size);
                free(buf);
                extracted = true;

                SceDateTime sce_time;
                time_t time;
                sceRtcGetCurrentClockLocalTime(&sce_time);
                sceRtcGetTime_t(&sce_time, &time);
                zip_cache[crc32] = {time, cache_name};

                zip_cache.CheckZipCacheSize();
                zip_cache.Save();
            }
        }
        zip_entry_close(zip);
    }
    zip_close(zip);

    return extracted ? zip_cache[crc32].name.c_str() : NULL;
}
