#ifndef __M_EMU_ZIP_H__
#define __M_EMU_ZIP_H__

#include <stdint.h>

int Archive_LoadCacheConfig();
int Archive_SaveCacheConfig();

int ZIP_GetRomMemory(const char *zip_path, void **buf, size_t *size);
int ZIP_GetRomPath(const char *zip_path, char *rom_path);

int SevenZ_GetRomMemory(const char *zip_path, void **buf, size_t *size);
int SevenZ_GetRomPath(const char *zip_path, char *rom_path);

#endif