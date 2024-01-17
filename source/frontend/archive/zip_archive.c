#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <psp2/io/fcntl.h>

#include "minizip/zip.h"
#include "config.h"
#include "file.h"
#include "utils.h"

static struct zip_t *current_zip = NULL;

int ZIP_OpenRom(const char *archive_path, uint32_t *crc, char *name)
{
    if (current_zip)
        zip_close(current_zip);

    current_zip = zip_open(archive_path, 0, 'r');
    if (!current_zip)
    {
        AppLog("[ZIP] ZIP_OpenRom failed: cannot open zip file!\n");
        return -1;
    }

    size_t total = zip_entries_total(current_zip);
    size_t i;
    for (i = 0; i < total; i++)
    {
        if (zip_entry_openbyindex(current_zip, i) == 0)
        {
            if (zip_entry_isdir(current_zip) == 0)
            {
                const char *entry_name = zip_entry_name(current_zip);
                if (entry_name && IsValidFile(entry_name))
                {
                    if (crc)
                        *crc = zip_entry_crc32(current_zip);
                    if (name)
                        strcpy(name, entry_name);
                    AppLog("[ZIP] ZIP_OpenRom OK!\n");
                    return 1;
                }
            }
            zip_entry_close(current_zip);
        }
    }

    AppLog("[ZIP] ZIP_OpenRom failed: no valid rom found!\n");

    zip_close(current_zip);
    current_zip = NULL;

    return 0;
}

void ZIP_CloseRom()
{
    AppLog("[ZIP] ZIP_CloseRom.\n");
    if (current_zip)
    {
        zip_entry_close(current_zip);
        zip_close(current_zip);
        current_zip = NULL;
    }
}

int ZIP_ExtractRomMemory(void **buf, size_t *size)
{
    if (!current_zip)
        return -1;

    if (zip_entry_read(current_zip, buf, size) <= 0)
    {
        AppLog("[ZIP] ZIP_ExtractRomMemory failed!\n");
        return -1;
    }

    AppLog("[ZIP] ZIP_ExtractRomMemory OK!\n");
    return 0;
}

int ZIP_ExtractRom(const char *rom_name, char *rom_path)
{
    if (!current_zip)
        return -1;

    CreateFolder(CORE_CACHE_DIR);
    sprintf(rom_path, "%s/%s", CORE_CACHE_DIR, rom_name);

    if (zip_entry_fread(current_zip, rom_path) != 0)
    {
        AppLog("[ZIP] ZIP_ExtractRom failed!\n");
        sceIoRemove(rom_path);
        return -1;
    }

    AppLog("[ZIP] ZIP_ExtractRom OK!\n");
    return 0;
}
