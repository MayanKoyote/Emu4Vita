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

ArchiveRomDriver *Archive_GetDriver(const char *ext);

int Archive_CleanCache(int index);
int Archive_CleanCacheByPath(const char *path);
int Archive_CleanAllCaches();

int Archive_LoadCacheConfig();
int Archive_SaveCacheConfig();

int Archive_GetRomMemory(const char *archive_path, void **buf, size_t *size, ArchiveRomDriver *driver);
int Archive_GetRomPath(const char *archive_path, char *rom_path, ArchiveRomDriver *driver);

#endif
