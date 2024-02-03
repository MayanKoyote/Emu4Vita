#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "emu/emu_archive.h"
#include "archive_zip.h"
#include "archive_lib.h"

static LibarchiveObj *current_archive = NULL;

static int openRom(const char *archive_path, uint32_t *crc, char *name)
{
    Libarchive_CloseRom(current_archive);
    current_archive = Libarchive_OpenRom(archive_path, archive_read_support_format_zip, crc, name);
    return current_archive != NULL ? 1 : 0;
}

static int closeRom()
{
    Libarchive_CloseRom(current_archive);
    current_archive = NULL;
    return 0;
}

static int extractRomMemory(void **buf, size_t *size)
{
    return Libarchive_ExtractRomMemory(current_archive, buf, size);
}

static int extractRom(char *extract_path)
{
    return Libarchive_ExtractRom(current_archive, extract_path);
}

ArchiveRomDriver archive_zip_driver = {
    "zip",
    openRom,
    closeRom,
    extractRomMemory,
    extractRom,
};