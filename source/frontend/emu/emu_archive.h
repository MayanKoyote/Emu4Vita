#ifndef __M_EMU_ARCHIVE_H__
#define __M_EMU_ARCHIVE_H__

#include <stdint.h>

#define MAX_CACHE_SIZE 5

typedef struct
{
    uint32_t crc;               // crc32
    uint64_t ltime;             // 加载时间
    char name[MAX_NAME_LENGTH]; // rom名称
} ArchiveCacheEntry;

extern ArchiveCacheEntry archive_cache_entries[MAX_CACHE_SIZE];
extern int archive_cache_num;

int Archive_LoadCacheConfig();
int Archive_SaveCacheConfig();
int Archive_FindRomCache(uint32_t crc, const char *rom_name, char *rom_path);
int Archive_GetInsertCacheEntriesIndex();

#include "archive/zip_archive.h"
#include "archive/7z_archive.h"

#endif