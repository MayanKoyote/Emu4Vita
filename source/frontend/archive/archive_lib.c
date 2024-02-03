#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <psp2/io/fcntl.h>

#include <archive.h>
#include <archive_entry.h>

#include "archive_lib.h"
#include "file.h"
#include "utils.h"

#define ARCHIVE_BLOCK_SIZE 10240
#define ARCHIVE_BUF_SIZE (128 * 1024)

LibarchiveObj *Libarchive_OpenRom(const char *archive_path, int format_code, uint32_t *crc, char *name)
{
    LibarchiveObj *obj = (LibarchiveObj *)calloc(1, sizeof(LibarchiveObj));
    if (!obj)
    {
        AppLog("[ARCHIVE] Libarchive_OpenRom failed: cannot alloc obj buf!\n");
        goto FAILED;
    }

    obj->archive = archive_read_new();
    if (!obj->archive)
    {
        AppLog("[ARCHIVE] Libarchive_OpenRom failed: cannot creat archive obj!\n");
        goto FAILED;
    }

    archive_read_support_format_by_code(obj->archive, format_code);

    if (archive_read_open_filename(obj->archive, archive_path, ARCHIVE_BLOCK_SIZE) != ARCHIVE_OK)
    {
        AppLog("[ARCHIVE] Libarchive_OpenRom failed: cannot open archive file!\n");
        goto FAILED;
    }

    while (archive_read_next_header(obj->archive, &obj->entry) == ARCHIVE_OK)
    {
        const char *entry_name = archive_entry_pathname_utf8(obj->entry);
        AppLog("[ARCHIVE] Libarchive_OpenRom: entry_name = %s\n", entry_name);
        if (entry_name && IsValidFile(entry_name))
        {
            if (name)
                strcpy(name, entry_name);
            if (crc)
                *crc = archive_entry_crc32(obj->entry);
            AppLog("[ARCHIVE] Libarchive_OpenRom OK!\n");
            return obj;
        }
    }

    AppLog("[ARCHIVE] Libarchive_OpenRom failed: no valid rom found!\n");

FAILED:
    Libarchive_CloseRom(obj);
    return NULL;
}

void Libarchive_CloseRom(LibarchiveObj *obj)
{
    if (obj)
    {
        if (obj->archive)
            archive_read_free(obj->archive);
        free(obj);
        AppLog("[ARCHIVE] Libarchive_CloseRom OK.\n");
    }
}

int Libarchive_ExtractRomMemory(LibarchiveObj *obj, void **buf, size_t *size)
{
    int ret = -1;

    if (!obj || !obj->archive || !obj->entry)
        goto FAILED;

    *size = archive_entry_size(obj->entry);
    *buf = malloc(*size);
    if (!*buf)
    {
        AppLog("[ARCHIVE] Libarchive_ExtractRomMemory failed: cannot alloc buf!\n");
        return -1;
    }

    archive_seek_data(obj->archive, 0, SEEK_SET);
    ret = archive_read_data(obj->archive, *buf, *size);
    if (ret != *size)
        goto FAILED;

END:
    if (ret < 0)
        AppLog("[ARCHIVE] Libarchive_ExtractRomMemory failed!\n");
    else
        AppLog("[ARCHIVE] Libarchive_ExtractRomMemory OK!\n");

    return ret;

FAILED:
    ret = -1;
    if (*buf)
        free(*buf);
    *size = 0;
    goto END;
}

int Libarchive_ExtractRom(LibarchiveObj *obj, const char *extract_path)
{
    int ret = -1;
    SceUID fd = -1;
    char *buf = NULL;

    if (!obj || !obj->archive || !obj->entry)
        goto FAILED;

    fd = sceIoOpen(extract_path, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
    if (fd < 0)
    {
        AppLog("[ARCHIVE] Libarchive_ExtractRom failed: cannot open file for write!\n");
        goto FAILED;
    }

    buf = (char *)malloc(ARCHIVE_BUF_SIZE);
    if (!buf)
    {
        AppLog("[ARCHIVE] Libarchive_ExtractRom failed: cannot alloc buf!\n");
        goto FAILED;
    }

    archive_seek_data(obj->archive, 0, SEEK_SET);
    while (1)
    {
        int read = archive_read_data(obj->archive, buf, ARCHIVE_BUF_SIZE);
        if (read < 0)
        {
            AppLog("[ARCHIVE] Libarchive_ExtractRom failed: archive_read_data error!\n");
            goto FAILED;
        }

        if (read == 0)
            break;

        int written = sceIoWrite(fd, buf, read);
        if (written != read)
        {
            AppLog("[ARCHIVE] Libarchive_ExtractRom failed: write file error!\n");
            goto FAILED;
        }
    }
    ret = 0;

END:
    if (ret < 0)
        AppLog("[ARCHIVE] Libarchive_ExtractRom failed!\n");
    else
        AppLog("[ARCHIVE] Libarchive_ExtractRom OK!\n");
    if (buf)
        free(buf);
    if (fd >= 0)
        sceIoClose(fd);
    return ret;

FAILED:
    ret = -1;
    sceIoRemove(extract_path);
    goto END;
}
