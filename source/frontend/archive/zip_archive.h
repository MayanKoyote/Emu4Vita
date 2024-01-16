#ifndef __M_ZIP_ARCHIVE_H__
#define __M_ZIP_ARCHIVE_H__

int ZIP_OpenRom(const char *archive_path, uint32_t *crc, char *name);
void ZIP_CloseRom();
int ZIP_ExtractRomMemory(void **buf, size_t *size);
int ZIP_ExtractRom(const char *rom_name, char *rom_path);

#endif
