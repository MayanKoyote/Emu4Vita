#ifndef __M_ZIP_ARCHIVE_H__
#define __M_ZIP_ARCHIVE_H__

int ZIP_GetRomMemory(const char *zip_path, void **buf, size_t *size);
int ZIP_GetRomPath(const char *zip_path, char *rom_path);

#endif