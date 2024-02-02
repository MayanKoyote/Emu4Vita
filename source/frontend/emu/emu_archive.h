#ifndef __M_EMU_ARCHIVE_H__
#define __M_EMU_ARCHIVE_H__

#include <stdint.h>

enum ArchiveMode
{
    ARCHIVE_MODE_NO = 0,
    ARCHIVE_MODE_ZIP,
    ARCHIVE_MODE_7Z,
};

#define WANT_SAVE_MEM_ROM_CACHE

int Archive_GetMode(const char *path);

int Archive_CleanCache(int index);
int Archive_CleanCacheByPath(const char *path);
int Archive_CleanAllCaches();

int Archive_LoadCacheConfig();
int Archive_SaveCacheConfig();

int Archive_GetRomMemory(const char *archive_path, void **buf, size_t *size, int archive_mode);
int Archive_GetRomPath(const char *archive_path, char *rom_path, int archive_mode);

#ifdef WANT_SAVE_MEM_ROM_CACHE
int Archive_SaveMemRomCache(uint32_t crc, const char *rom_name, const void *buf, size_t size);
int Archive_WaitThreadEnd();
#endif

#endif