#ifndef __M_7Z_ARCHIVE_H__
#define __M_7Z_ARCHIVE_H__

int SevenZ_GetRomMemory(const char *zip_path, void **buf, size_t *size);
int SevenZ_GetRomPath(const char *zip_path, char *rom_path);

#endif