#include <stdio.h>
#include <psp2/io/dirent.h>
#include <psp2/io/stat.h>
#include "config.h"
#include "zip_cache.h"
#include "zip/zip.h"
#include "list/file_list.h"

#define CACHE_NUM 5
#define CHECK_SIZE 0x20

struct CachedItem
{
    uint32_t hash;
    SceDateTime time;
};

struct CachedItem zip_cache[CACHE_NUM] = {0};

void InitZipCache()
{
    SceUID dfd = sceIoDopen(CORE_ZIPCACHE_DIR);
    if (dfd < 0)
    {
        sceIoMkdir(CORE_ZIPCACHE_DIR, SCE_S_IFDIR | SCE_S_IWUSR);
        return;
    }
    else
    {
        RefreshZipCache();
    }
}

void RefreshZipCache()
{
    LinkedList *file_list = NULL;
}

uint32_t Hash(uint8_t *buf, size_t size)
{
    uint32_t hash = 0x12345678;
    uint8_t *end = buf + size;
    while (buf != end)
    {
        hash = ((hash << 5) + hash) + *buf++;
    }
    return hash;
}

/*
hash 计算方法：文件大小 ^ 前 CHECK_SIZE 字节的hash ^ 末尾 CHECK_SIZE 字节的hash
返回值为 0 为无效
*/
uint32_t CalcZipHash(const char *name)
{
    uint32_t hash = 0;
    uint8_t buf[CHECK_SIZE];

    FILE *fp = fopen(name, "rb");
    if (fp)
    {
        fseek(fp, 0, SEEK_END);
        hash = ftell(fp);
        if (hash < CHECK_SIZE * 2)
        {
            hash = 0;
        }
        else
        {
            fseek(fp, 0, SEEK_SET);
            fread(buf, CHECK_SIZE, 1, fp);
            hash ^= Hash(buf, CHECK_SIZE);

            fseek(fp, -CHECK_SIZE, SEEK_END);
            fread(buf, CHECK_SIZE, 1, fp);
            hash ^= Hash(buf, CHECK_SIZE);
        }
        fclose(fp);
    }

    return hash;
}
