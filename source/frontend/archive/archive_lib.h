#ifndef __M_ARCHIVE_LIB_H__
#define __M_ARCHIVE_LIB_H__

#include <stdint.h>

#include <archive.h>
#include <archive_entry.h>

typedef struct
{
    struct archive *archive;
    struct archive_entry *entry;
} LibarchiveObj;

LibarchiveObj *Libarchive_OpenRom(const char *archive_path, int format_code, uint32_t *crc, char *name);
void Libarchive_CloseRom(LibarchiveObj *obj);
int Libarchive_ExtractRomMemory(LibarchiveObj *obj, void **buf, size_t *size);
int Libarchive_ExtractRom(LibarchiveObj *obj, const char *extract_path);

#endif
