#ifndef __M_EMU_ARCHIVE_H__
#define __M_EMU_ARCHIVE_H__

#include <stdint.h>

typedef struct
{
    char *extension;
    int (*openRom)(const char *archive_path, uint32_t *crc, char *name);
    int (*closeRom)();
    int (*extractRomMemory)(void **buf, size_t *size);
    int (*extractRom)(char *extract_path);
} ArchiveRomDriver;

// #define WANT_SAVE_MEM_ROM_CACHE

int Archive_GetDriverIndex(const char *ext);
ArchiveRomDriver *Archive_GetDriver(int index);

int Archive_CleanCache(int index);
int Archive_CleanCacheByPath(const char *path);
int Archive_CleanAllCaches();

int Archive_LoadCacheConfig();
int Archive_SaveCacheConfig();

int Archive_GetRomMemory(const char *archive_path, void **buf, size_t *size, ArchiveRomDriver *driver);
int Archive_GetRomPath(const char *archive_path, char *rom_path, ArchiveRomDriver *driver);

#ifdef WANT_SAVE_MEM_ROM_CACHE
int Archive_SaveMemRomCache(uint32_t crc, const char *rom_name, const void *buf, size_t size);
int Archive_WaitThreadEnd();
#endif

#endif