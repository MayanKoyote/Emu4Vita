#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/io/fcntl.h>
#include <7zTypes.h>
#include <7z.h>
#include <7zFile.h>
#include <7zAlloc.h>
#include <7zCrc.h>
#include <string.h>
#include <encodings/utf.h>
#include "file.h"
#include "utils.h"
#include "config.h"
#include "emu/emu.h"

#define SEVENZIP_LOOKTOREAD_BUF_SIZE (1 << 14)

typedef struct
{
    int inited;
    int index;
    CFileInStream file_stream;
    CLookToRead2 look_stream;
    CSzArEx db;
    char name[MAX_PATH_LENGTH];
} SevenContext;

static SevenContext current_7z = {0};
static ISzAlloc alloc_imp = {SzAlloc, SzFree};
static ISzAlloc alloc_temp_imp = {SzAllocTemp, SzFreeTemp};

void Init7z()
{
    current_7z.look_stream.buf = malloc(SEVENZIP_LOOKTOREAD_BUF_SIZE);
    if (current_7z.look_stream.buf)
    {
        current_7z.look_stream.bufSize = SEVENZIP_LOOKTOREAD_BUF_SIZE;
    }

    CrcGenerateTable();

    current_7z.inited = 1;
}

void Deinit7z()
{
    SzArEx_Free(&current_7z.db, &alloc_imp);

    if (current_7z.file_stream.file.file)
        File_Close(&current_7z.file_stream.file);

    if (current_7z.look_stream.buf)
        free(current_7z.look_stream.buf);

    memset(&current_7z, 0, sizeof(current_7z));
}

static int
SevenZ_GetRomEntry(const char *archive_path)
{
    if (current_7z.inited)
    {
        Deinit7z();
    }

    Init7z();

    if (InFile_Open(&current_7z.file_stream.file, archive_path) != 0)
    {
        Deinit7z();
        return -1;
    }

    FileInStream_CreateVTable(&current_7z.file_stream);
    LookToRead2_CreateVTable(&current_7z.look_stream, False);
    current_7z.look_stream.realStream = &current_7z.file_stream.vt;
    LookToRead2_INIT(&current_7z.look_stream);

    CSzArEx *db = &current_7z.db;
    SzArEx_Init(db);
    if (SzArEx_Open(db, &current_7z.look_stream.vt, &alloc_imp, &alloc_temp_imp) != SZ_OK)
    {
        Deinit7z();
        return -1;
    }

    uint16_t name_u16[MAX_PATH_LENGTH];
    for (int i = 0; i < db->NumFiles; i++)
    {
        if (SzArEx_IsDir(db, i))
            continue;

        size_t u16_size = SzArEx_GetFileNameUtf16(db, i, name_u16);
        size_t u8_size = MAX_PATH_LENGTH;
        utf16_conv_utf8((uint8_t *)current_7z.name, &u8_size, name_u16, u16_size);
        if (IsValidFile(current_7z.name))
        {
            current_7z.index = i;
            AppLog("[7Z] GetRomEntry OK!\n");
            return 1;
        }
    }

    Deinit7z();

    return -1;
}

static int SevernZ_FindRomCache(const char *rom_name, char *rom_path)
{
    if (!current_7z.inited)
        return -1;

    return Archive_FindRomCache(current_7z.db.CRCs.Vals[current_7z.index], rom_name, rom_path);
}

static int SevernZ_ExtractRomCache(const char *rom_name, char *rom_path)
{
    if (!current_7z.inited)
        return -1;

    CreateFolder(CORE_CACHE_DIR);
    sprintf(rom_path, "%s/%s", CORE_CACHE_DIR, rom_name);

    uint32_t block_index = 0xFFFFFFFF;
    size_t output_size = 0;
    size_t offset = 0;
    size_t outsize_processed = 0;
    uint8_t *output = NULL;
    int index = -1;
    SRes res = SzArEx_Extract(&current_7z.db, &current_7z.look_stream.vt, current_7z.index, &block_index,
                              &output, &output_size, &offset, &outsize_processed, &alloc_imp, &alloc_temp_imp);
    if (res != SZ_OK)
    {
        AppLog("[7Z] Extract failed: %d\n", res);
        goto EXTRACT_CACHE_END;
    }

    if (WriteFile(rom_path, output + offset, outsize_processed) < 0)
    {
        AppLog("[7Z] Write file failed: %s\n", rom_path);
        goto EXTRACT_CACHE_END;
    }

    index = Archive_InsertRomCache(current_7z.db.CRCs.Vals[current_7z.index], rom_name);

    AppLog("[7Z] ExtractRomCache OK: %d, %s\n", index, rom_name);

EXTRACT_CACHE_END:
    if (output)
        IAlloc_Free(&alloc_imp, output);

    return index;
}

static int SevenZ_ExtractRomMemory(void **buf, size_t *size)
{
    if (!current_7z.inited)
        return -1;

    AppLog("[7Z] Start extacting\n");

    uint32_t block_index = 0xFFFFFFFF;
    size_t output_size = 0;
    size_t offset = 0;
    uint8_t *output = NULL;
    *size = 0;
    SRes res = SzArEx_Extract(&current_7z.db, &current_7z.look_stream.vt, current_7z.index, &block_index,
                              &output, &output_size, &offset, size, &alloc_imp, &alloc_temp_imp);
    if (res != SZ_OK)
    {
        AppLog("[7Z] Extract failed: %d\n", res);
        goto EXTRACT_MEM_END;
    }

    *buf = malloc(*size);
    if (*buf)
    {
        memcpy(*buf, output + offset, *size);
    }
    else
    {
        *size = 0;
    }

EXTRACT_MEM_END:
    if (output)
        IAlloc_Free(&alloc_imp, output);

    AppLog("[7Z] End extracting\n");

    return *size > 0;
}

int SevenZ_GetRomMemory(const char *archive_path, void **buf, size_t *size)
{
    if (SevenZ_GetRomEntry(archive_path) <= 0)
        return -1;

    char rom_name[MAX_NAME_LENGTH];
    MakeBaseName(rom_name, archive_path, sizeof(rom_name));
    const char *ext = strrchr(current_7z.name, '.');
    if (ext)
        strcat(rom_name, ext);

    char rom_path[MAX_NAME_LENGTH];
    int index = SevernZ_FindRomCache(rom_name, rom_path);
    if (index >= 0)
    {
        *size = AllocateReadFile(rom_path, buf);
    }
    else
    {
        SevenZ_ExtractRomMemory(buf, size);
        if (*size > 0)
            Archive_AsyncWriteRomCache(current_7z.db.CRCs.Vals[current_7z.index], rom_name, *buf, *size);
    }

    Deinit7z();

    if (*size <= 0)
    {
        AppLog("[7Z] GetRomMemory failed!\n");
        return -1;
    }
    else
    {
        AppLog("[7Z] GetRomMemory OK!\n");
        return 0;
    }
}

int SevenZ_GetRomPath(const char *archive_path, char *rom_path)
{
    if (SevenZ_GetRomEntry(archive_path) <= 0)
        return -1;

    char rom_name[MAX_NAME_LENGTH];
    MakeBaseName(rom_name, archive_path, sizeof(rom_name));
    const char *ext = strrchr(current_7z.name, '.');
    if (ext)
        strcat(rom_name, ext);

    int index = SevernZ_FindRomCache(rom_name, rom_path);
    if (index < 0)
        index = SevernZ_ExtractRomCache(rom_name, rom_path);

    Deinit7z();

    if (index < 0)
    {
        AppLog("[7Z] GetRomPath failed!\n");
        return -1;
    }

    // 设置rom加载时间为当前线程时间
    archive_cache_entries[index].ltime = sceKernelGetProcessTimeWide();
    AppLog("[7Z] GetRomPath OK\n");
    return 0;
}