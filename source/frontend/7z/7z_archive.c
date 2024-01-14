#include <stdint.h>
#include <stdlib.h>
#include <7zTypes.h>
#include <7z.h>
#include <7zFile.h>
#include <7zAlloc.h>
#include <string.h>

typedef struct
{
    int inited;
    CFileInStream file_stream;
    CLookToRead2 look_stream;
    CSzArEx db;
    uint8_t *buf;
} SevenContext;

static SevenContext current_7z = {0};
static ISzAlloc allocImp = {SzAlloc, SzFree};
static ISzAlloc allocTempImp = {SzAllocTemp, SzFreeTemp};

void Init7z()
{
    // SzArEx_Init(&current_7z.db);
    // current_7z.inited = 1;
}

void Deinit7z()
{
    // SzArEx_Free(&current_7z.db, &allocImp);
    // memset(&current_7z, 0, sizeof(current_7z));
}

static int
SevenZ_GetRomEntry(const char *sevenz_path)
{
    if (current_7z.inited)
    {
        Deinit7z();
    }

    if (InFile_Open(&current_7z.file_stream.file, sevenz_path) != 0)
    {
        return -1;
    }

    FileInStream_CreateVTable(&current_7z.file_stream);
    LookToRead2_CreateVTable(&current_7z.look_stream, False);
    current_7z.look_stream.realStream = &current_7z.file_stream.vt;
    LookToRead2_Init(&current_7z.look_stream);

    if (SzArEx_Open(&current_7z.db, &current_7z.look_stream.vt, &allocImp, &allocTempImp) != SZ_OK)
    {
    }
}

// static int SevernZ_FindRomCache(const char *rom_name, char *rom_path)
// {
//     return 0;
// }

int SevenZ_GetRomMemory(const char *zip_path, void **buf, size_t *size)
{
    return 0;
}

int SevenZ_GetRomPath(const char *zip_path, char *rom_path)
{
    return 0;
}