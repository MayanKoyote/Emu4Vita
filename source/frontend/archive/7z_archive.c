#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/io/fcntl.h>
#include <7zip/7zTypes.h>
#include <7zip/7z.h>
#include <7zip/7zFile.h>
#include <7zip/7zAlloc.h>
#include <7zip/7zCrc.h>
#include <string.h>
#include <encodings/utf.h>
#include "file.h"
#include "utils.h"
#include "config.h"
#include "emu/emu.h"
#include "7z_archive.h"

#define SEVENZIP_LOOKTOREAD_BUF_SIZE (1 << 14)

typedef struct
{
    int inited;
    int crc32_init;
    int index;
    CFileInStream file_stream;
    CLookToRead2 look_stream;
    CSzArEx db;
} SevenContext;

ISzAlloc alloc_imp = {SzAlloc, SzFree};
ISzAlloc alloc_temp_imp = {SzAllocTemp, SzFreeTemp};

static SevenContext current_7z = {0};

int SevenZ_OpenRom(const char *archive_path, uint32_t *crc, char *name)
{
    if (current_7z.inited)
    {
        SevenZ_CloseRom();
    }

    if (!current_7z.crc32_init)
    {
        CrcGenerateTable();
        current_7z.crc32_init = 1;
    }

    if (InFile_Open(&current_7z.file_stream.file, archive_path) != 0)
    {
        AppLog("[7Z] SevenZ_OpenRom failed: cannot open 7z file!\n");
        return -1;
    }

    current_7z.look_stream.buf = malloc(SEVENZIP_LOOKTOREAD_BUF_SIZE);
    if (current_7z.look_stream.buf)
        current_7z.look_stream.bufSize = SEVENZIP_LOOKTOREAD_BUF_SIZE;

    current_7z.inited = 1;

    FileInStream_CreateVTable(&current_7z.file_stream);
    LookToRead2_CreateVTable(&current_7z.look_stream, False);
    current_7z.look_stream.realStream = &current_7z.file_stream.vt;
    LookToRead2_INIT(&current_7z.look_stream);

    CSzArEx *db = &current_7z.db;
    SzArEx_Init(db);
    if (SzArEx_Open(db, &current_7z.look_stream.vt, &alloc_imp, &alloc_temp_imp) != SZ_OK)
    {
        SevenZ_CloseRom();
        return -1;
    }

    uint16_t name_u16[MAX_PATH_LENGTH];
    char name_u8[MAX_PATH_LENGTH];
    if (!name)
        name = name_u8;
    for (int i = 0; i < db->NumFiles; i++)
    {
        if (SzArEx_IsDir(db, i))
            continue;

        size_t u16_size = SzArEx_GetFileNameUtf16(db, i, name_u16);
        size_t u8_size = MAX_PATH_LENGTH;
        utf16_conv_utf8((uint8_t *)name, &u8_size, name_u16, u16_size);
        if (IsValidFile(name))
        {
            current_7z.index = i;
            if (crc)
                *crc = db->CRCs.Vals[i];

            AppLog("[7Z] SevenZ_OpenRom OK!\n");
            return 1;
        }
    }

    AppLog("[7Z] SevenZ_OpenRom failed: no valid rom found!\n");

    SevenZ_CloseRom();

    return -1;
}

void SevenZ_CloseRom()
{
    AppLog("[7Z] SevenZ_CloseRom.\n");

    SzArEx_Free(&current_7z.db, &alloc_imp);

    if (current_7z.file_stream.file.file)
        File_Close(&current_7z.file_stream.file);

    if (current_7z.look_stream.buf)
        free(current_7z.look_stream.buf);

    memset(&current_7z, 0, sizeof(current_7z));
}

int SevenZ_ExtractRomMemory(void **buf, size_t *size)
{
    if (!current_7z.inited)
        return -1;

    uint32_t block_index = 0xFFFFFFFF;
    size_t output_size = 0;
    size_t offset = 0;
    uint8_t *output = NULL;
    *size = 0;
    SRes res = SzArEx_Extract(&current_7z.db, &current_7z.look_stream.vt, current_7z.index, &block_index,
                              &output, &output_size, &offset, size, &alloc_imp, &alloc_temp_imp);
    if (res != SZ_OK)
    {
        AppLog("[7Z] SevenZ_ExtractRomMemory failed!");
        goto EXTRACT_MEM_END;
    }

    *buf = malloc(*size);
    if (*buf)
        memcpy(*buf, output + offset, *size);
    else
        *size = 0;

    AppLog("[7Z] SevenZ_ExtractRomMemory OK\n");

EXTRACT_MEM_END:
    if (output)
        IAlloc_Free(&alloc_imp, output);

    return *size > 0 ? 0 : -1;
}

int SevenZ_ExtractRom(const char *rom_name, char *rom_path)
{
    if (!current_7z.inited)
        return -1;

    CreateFolder(CORE_CACHE_DIR);

    void *buf;
    size_t size;
    if (SevenZ_ExtractRomMemory(&buf, &size) <= 0)
    {
        AppLog("[7Z] SevenZ_ExtractRom failed!\n");
        return -1;
    }

    sprintf(rom_path, "%s/%s", CORE_CACHE_DIR, rom_name);
    int res = WriteFile(rom_path, buf, size) > 0 ? 0 : -1;
    free(buf);

    AppLog(res == 0 ? "[7Z] SevenZ_ExtractRom OK\n" : "[7Z] SevenZ_ExtractRom write file failed!\n");
    return res;
}
