#include <stdlib.h>
#include <string.h>
#include <archive.h>
#include <archive_entry.h>
#include "config.h"
#include "file.h"
#include "utils.h"
#include "archive_rom.h"

#define ARCHIVE_BLOCK_SIZE 10240
#define ARCHIVE_BUF_SIZE 0x800 * 0x1000

static struct archive *current_archive = NULL;
static struct archive_entry *current_entry = NULL;

int ArchiveRom_Open(const char *archive_path, uint32_t *crc, char *name)
{
    if (current_archive)
        archive_read_free(current_archive);

    current_archive = archive_read_new();
    if (!current_archive)
        return -1;

    archive_read_support_filter_all(current_archive);
    archive_read_support_format_all(current_archive);

    // archive_read_support_format_7zip(current_archive);
    // archive_read_support_format_zip(current_archive);

    if (archive_read_open_filename(current_archive, archive_path, ARCHIVE_BLOCK_SIZE) != ARCHIVE_OK)
    {
        AppLog("[ARCHIVE] Archive_OpenRom failed: cannot open file!\n");
        goto FAILED;
    }

    while (archive_read_next_header(current_archive, &current_entry) == ARCHIVE_OK)
    {
        const char *entry_name = archive_entry_pathname_utf8(current_entry);
        AppLog("[ARCHIVE] Archive_OpenRom: entry_name = %s\n", entry_name);
        if (entry_name && IsValidFile(entry_name))
        {
            if (name)
                strcpy(name, entry_name);
            if (crc)
                *crc = archive_entry_crc32(current_entry);
            AppLog("[ARCHIVE] Archive_OpenRom OK!\n");
            return 1;
        }
    }

    AppLog("[ARCHIVE] Archive_OpenRom failed: no valid rom found!\n");

FAILED:
    archive_read_free(current_archive);
    current_archive = NULL;
    return -1;
}

void ArchiveRom_Close()
{
    if (current_archive)
    {
        archive_read_free(current_archive);
        current_archive = NULL;
    }
}

int ArchiveRom_ExtractToMemory(void **buf, size_t *size)
{
    if (!current_archive)
        return -1;

    *size = archive_entry_size(current_entry);
    *buf = malloc(*size);
    if (!*buf)
    {
        AppLog("[ARCHIVE] Archive_ExtractRom failed: cannot alloc buf!\n");
        return -1;
    }

    return archive_read_data(current_archive, *buf, *size) == *size ? 0 : -1;
}

int ArchiveRom_Extract(const char *rom_name, char *rom_path)
{
    if (!current_archive)
        return -1;

    CreateFolder(CORE_CACHE_DIR);
    sprintf(rom_path, "%s/%s", CORE_CACHE_DIR, rom_name);

    FILE *fp = fopen(rom_path, "wb");
    if (!fp)
    {
        AppLog("[ARCHIVE] Failed to open file for writing: %s\n", rom_path);
        return -1;
    }

    int ret = -1;
    char *buf = malloc(ARCHIVE_BUF_SIZE);
    if (!buf)
    {
        AppLog("[ARCHIVE] Archive_ExtractRom failed: cannot alloc buf!\n");
        goto END;
    }

    ssize_t size;
    do
    {
        size = archive_read_data(current_archive, buf, ARCHIVE_BUF_SIZE);
        if (size > 0)
            fwrite(buf, size, 1, fp);
        else if (size == 0)
            break;
        else
            goto END;
    } while (size > 0);
    ret = 0;

    AppLog("[ARCHIVE] Archive_ExtractRom OK!\n");

END:
    if (buf)
        free(buf);
    fclose(fp);

    return ret;
}
