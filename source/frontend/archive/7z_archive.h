#ifndef __M_7Z_ARCHIVE_H__
#define __M_7Z_ARCHIVE_H__

int SevenZ_OpenRom(const char *archive_path, uint32_t *crc, char *name);
void SevenZ_CloseRom();
int SevenZ_ExtractRomMemory(void **buf, size_t *size);
int SevenZ_ExtractRom(const char *rom_name, char *rom_path);

#endif