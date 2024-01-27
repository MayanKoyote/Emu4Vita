#ifndef __M_ARCHIVE_ROM_H__
#define __M_ARCHIVE_ROM_H__
#include <stdint.h>

int ArchiveRom_Open(const char *archive_path, uint32_t *crc, char *name);
void ArchiveRom_Close();
int ArchiveRom_ExtractToMemory(void **buf, size_t *size);
int ArchiveRom_Extract(const char *rom_name, char *rom_path);

#endif