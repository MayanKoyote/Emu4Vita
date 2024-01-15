#include <stdio.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/io/fcntl.h>
#include "zip/zip.h"
#include "emu/emu.h"
#include "config.h"
#include "utils.h"
#include "zip_archive.h"

static struct zip_t *current_zip = NULL;

static int ZIP_FindRomCache(const char *rom_name, char *rom_path)
{
    if (!current_zip)
        return -1;

    uint32_t crc = zip_entry_crc32(current_zip);

    return Archive_FindRomCache(crc, rom_name, rom_path);
}

static int ZIP_ExtractRomCache(const char *rom_name, char *rom_path)
{
    if (!current_zip)
        return -1;

    CreateFolder(CORE_CACHE_DIR);
    sprintf(rom_path, "%s/%s", CORE_CACHE_DIR, rom_name);

    if (zip_entry_fread(current_zip, rom_path) != 0)
    {
        sceIoRemove(rom_path);
        AppLog("[ZIP] ExtractRomCache failed: %s\n", rom_name);
        return -1;
    }

    int index = Archive_GetInsertCacheEntriesIndex(); // 获取新条目插入位置

    if (archive_cache_num >= MAX_CACHE_SIZE) // 缓存条目已达到最大数
    {
        // 删除要替换的旧条目指向的rom文件
        char tmp_path[MAX_PATH_LENGTH];
        sprintf(tmp_path, "%s/%s", CORE_CACHE_DIR, archive_cache_entries[index].name);
        sceIoRemove(tmp_path);
    }
    else
    {
        archive_cache_num++;
    }

    // 设置新条目的crc和name，ltime由ZIP_GetRomPath函数设定
    memset(&archive_cache_entries[index], 0, sizeof(archive_cache_entries[index]));
    archive_cache_entries[index].crc = zip_entry_crc32(current_zip);
    strcpy(archive_cache_entries[index].name, rom_name);

    Archive_SaveCacheConfig();

    AppLog("[ZIP] ExtractRomCache OK: %d, %s\n", index, rom_name);
    return index;
}

static int ZIP_GetRomEntry(const char *zip_path)
{
    if (current_zip)
        zip_close(current_zip);

    current_zip = zip_open(zip_path, 0, 'r');
    if (!current_zip)
        return -1;

    size_t total = zip_entries_total(current_zip);
    size_t i;
    for (i = 0; i < total; i++)
    {
        if (zip_entry_openbyindex(current_zip, i) == 0)
        {
            const char *entry_name = zip_entry_name(current_zip);
            if (IsValidFile(entry_name))
            {
                AppLog("[ZIP] GetRomEntry OK!\n");
                return 1;
            }
            zip_entry_close(current_zip);
        }
    }

    if (current_zip)
    {
        zip_close(current_zip);
        current_zip = NULL;
    }
    AppLog("[ZIP] GetRomEntry failed!\n");
    return 0;
}

int ZIP_GetRomMemory(const char *zip_path, void **buf, size_t *size)
{
    if (ZIP_GetRomEntry(zip_path) <= 0)
        return -1;

    ssize_t result = zip_entry_read(current_zip, buf, size);
    if (current_zip)
    {
        zip_entry_close(current_zip);
        zip_close(current_zip);
        current_zip = NULL;
    }

    if (result <= 0)
    {
        AppLog("[ZIP] GetRomMemory failed!\n");
        return -1;
    }
    else
    {
        AppLog("[ZIP] GetRomMemory OK!\n");
        return 0;
    }
}

int ZIP_GetRomPath(const char *zip_path, char *rom_path)
{
    if (ZIP_GetRomEntry(zip_path) <= 0)
        return -1;

    // 缓存文件名：例GBA: game1.zip ==> game1.gba（忽略压缩包内的rom文件名，以zip文件名为rom名）
    char rom_name[MAX_NAME_LENGTH];
    MakeBaseName(rom_name, zip_path, sizeof(rom_name));
    const char *entry_name = zip_entry_name(current_zip);
    const char *ext = strrchr(entry_name, '.');
    if (ext)
        strcat(rom_name, ext);

    int index = ZIP_FindRomCache(rom_name, rom_path);
    if (index < 0)
        index = ZIP_ExtractRomCache(rom_name, rom_path);

    if (current_zip)
    {
        zip_entry_close(current_zip);
        zip_close(current_zip);
        current_zip = NULL;
    }

    if (index < 0)
    {
        AppLog("[ZIP] GetRomPath failed!\n");
        return -1;
    }

    // 设置rom加载时间为当前线程时间
    archive_cache_entries[index].ltime = sceKernelGetProcessTimeWide();
    AppLog("[ZIP] GetRomPath OK: %s\n", rom_path);
    return 0;
}