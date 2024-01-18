#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <psp2/kernel/processmgr.h>
#include <psp2/io/fcntl.h>
#include <archive.h>
#include <archive_entry.h>

#include "emu/emu.h"
#include "file.h"
#include "config.h"
#include "utils.h"
#include "utils_string.h"

typedef struct
{
    uint32_t crc;               // crc32
    uint64_t ltime;             // 加载时间
    char name[MAX_NAME_LENGTH]; // rom名称
} ArchiveEntry;

#define MAX_CACHE_SIZE 5
#define ARCHIVE_CACHE_CONFIG_PATH CORE_CACHE_DIR "/archive_cache.txt"
#define ARCHIVE_BLOCK_SIZE 10240
#define ARCHIVE_BUF_SIZE 0x800 * 0x1000

static ArchiveEntry archive_cache_entries[MAX_CACHE_SIZE];
static int archive_cache_num = 0;

static struct archive *current_archive = NULL;
static struct archive_entry *current_entry = NULL;

int Archive_LoadCacheConfig()
{
    memset(archive_cache_entries, 0, sizeof(archive_cache_entries));
    archive_cache_num = 0;

    void *buffer = NULL;
    int size = AllocateReadFile(ARCHIVE_CACHE_CONFIG_PATH, &buffer);
    if (size < 0)
        return size;

    int res = 0;
    char *p = (char *)buffer;

    // Skip UTF-8 bom
    uint32_t bom = 0xBFBBEF;
    if (memcmp(p, &bom, 3) == 0)
    {
        p += 3;
        size -= 3;
    }

    char path[MAX_PATH_LENGTH];
    char *line = NULL;
    char *name = NULL, *value = NULL;

    do
    {
        res = StringGetLine(p, size, &line);
        // printf("StringGetLine: line = %s\n", line);
        if (res > 0)
        {
            if (StringReadConfigLine(line, &name, &value) >= 0)
            {
                // printf("StringGetLine: name = %s, value = %s\n", name, value);
                sprintf(path, "%s/%s", CORE_CACHE_DIR, value);
                if (CheckFileExist(path))
                {
                    archive_cache_entries[archive_cache_num].crc = StringToHexdecimal(name);
                    archive_cache_entries[archive_cache_num].ltime = 0; // 加载时间初始化为0
                    strcpy(archive_cache_entries[archive_cache_num].name, value);
                    archive_cache_num++;
                }
            }

            if (line)
                free(line);
            line = NULL;
            if (name)
                free(name);
            name = NULL;
            if (value)
                free(value);
            value = NULL;

            size -= res;
            p += res;
        }
    } while (res > 0 && archive_cache_num < MAX_CACHE_SIZE);

    free(buffer);

    return 0;
}

int Archive_SaveCacheConfig()
{
    SceUID fd = sceIoOpen(ARCHIVE_CACHE_CONFIG_PATH, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
    if (fd < 0)
        return fd;

    int ret = 0;
    char string[MAX_CONFIG_LINE_LENGTH];
    int i;
    for (i = 0; i < archive_cache_num; i++)
    {
        snprintf(string, sizeof(string), "%08X=\"%s\"\n", archive_cache_entries[i].crc, archive_cache_entries[i].name);
        if ((ret = sceIoWrite(fd, string, strlen(string))) < 0)
            break;
    }

    sceIoClose(fd);
    if (ret < 0)
        sceIoRemove(ARCHIVE_CACHE_CONFIG_PATH);
    return ret;
}

static int Archive_FindRomCache(int rom_crc, const char *rom_name, char *rom_path)
{
    int i;
    for (i = 0; i < archive_cache_num; i++)
    {
        if (archive_cache_entries[i].crc == rom_crc && strcasecmp(archive_cache_entries[i].name, rom_name) == 0)
        {
            sprintf(rom_path, "%s/%s", CORE_CACHE_DIR, rom_name);
            AppLog("[ARCHIVE] FindRomCache OK: %d, %s\n", i, rom_name);
            return i;
        }
    }

    AppLog("[ARCHIVE] FindRomCache failed: %s\n", rom_name);
    return -1;
}

static int getInsertCacheEntriesIndex()
{
    if (archive_cache_num < MAX_CACHE_SIZE)
        return archive_cache_num;

    int index = 0;
    uint64_t ltime = archive_cache_entries[0].ltime;

    int i;
    for (i = 1; i < MAX_CACHE_SIZE; i++)
    {
        // 获取最小加载时间的缓存条目
        if (archive_cache_entries[i].ltime < ltime)
            index = i;
    }

    return index;
}

static int Archive_AddCacheEntry(int crc, const char *rom_name)
{
    int index = getInsertCacheEntriesIndex(); // 获取新条目插入位置

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

    memset(&archive_cache_entries[index], 0, sizeof(archive_cache_entries[index]));
    // 设置新条目的crc
    archive_cache_entries[index].crc = crc;
    // 设置新条目的name
    strcpy(archive_cache_entries[index].name, rom_name);
    // 设置新条目的ltime为当前线程时间
    archive_cache_entries[index].ltime = sceKernelGetProcessTimeWide();

    Archive_SaveCacheConfig();

    AppLog("[ARCHIVE] Archive_AddCacheEntry OK: %d, %s\n", index, rom_name);
    return index;
}

static int Archive_OpenRom(const char *archive_path, uint32_t *crc, char *name)
{
    if (current_archive)
        archive_read_free(current_archive);

    current_archive = archive_read_new();
    if (!current_archive)
        return -1;

    archive_read_support_filter_all(current_archive);
    archive_read_support_format_all(current_archive);

    if (archive_read_open_filename(current_archive, archive_path, ARCHIVE_BLOCK_SIZE) != ARCHIVE_OK)
    {
        AppLog("[ARCHIVE] Archive_OpenRom failed: cannot open file!\n");
        goto FAILED;
    }

    while (archive_read_next_header(current_archive, &current_entry) == ARCHIVE_OK)
    {
        const char *entry_name = archive_entry_pathname_utf8(current_entry);
        AppLog("%s\n", entry_name);
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

static void Archive_CloseRom()
{
    if (current_archive)
    {
        archive_read_free(current_archive);
        current_archive = NULL;
    }
}

int Archive_ExtractRomMemory(void **buf, size_t *size)
{
    if (!current_archive)
        return -1;

    *size = archive_entry_size(current_entry);
    *buf = malloc(*size);
    if (!*buf)
    {
        AppLog("[ARCHIVE] Archive_ExtractRom failed: connot alloc buf!\n");
        return -1;
    }

    return archive_read_data(current_archive, buf, *size) == *size ? 0 : -1;
}

int Archive_ExtractRom(const char *rom_name, char *rom_path)
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
        AppLog("[ARCHIVE] Archive_ExtractRom failed: connot alloc buf!\n");
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

int Archive_GetRomMemory(const char *archive_path, void **buf, size_t *size, int archive_mode)
{
    int ret = Archive_OpenRom(archive_path, NULL, NULL);
    if (ret <= 0)
        goto END;

    ret = Archive_ExtractRomMemory(buf, size);

    Archive_CloseRom();

    if (ret < 0)
        AppLog("[ARCHIVE] Archive_GetRomMemory failed!\n");
    else
        AppLog("[ARCHIVE] Archive_GetRomMemory OK!\n");
END:
    return ret;
}

int Archive_GetRomPath(const char *archive_path, char *rom_path, int archive_mode)
{
    int ret = -1;
    uint32_t rom_crc;
    char entry_name[MAX_PATH_LENGTH];
    ret = Archive_OpenRom(archive_path, &rom_crc, entry_name);

    if (ret <= 0)
        goto FAILED;

    // 缓存文件名：例GBA: game1.zip ==> game1.gba（忽略压缩包内的rom文件名，以zip文件名为rom名）
    char rom_name[MAX_NAME_LENGTH];
    MakeBaseName(rom_name, archive_path, sizeof(rom_name));
    const char *ext = strrchr(entry_name, '.');
    if (ext)
        strcat(rom_name, ext);

    int index = Archive_FindRomCache(rom_crc, rom_name, rom_path);
    if (index < 0)
    {
        // 未找到cache，执行解压
        ret = Archive_ExtractRom(rom_name, rom_path);

        if (ret < 0)
            goto FAILED;

        // 添加新的cache条目
        Archive_AddCacheEntry(rom_crc, rom_name);
    }
    else
    {
        // 更新ltime为当前线程时间
        archive_cache_entries[index].ltime = sceKernelGetProcessTimeWide();
    }

END:
    Archive_CloseRom();

    if (ret < 0)
        AppLog("[ARCHIVE] Archive_GetRomPath failed!\n");
    else
        AppLog("[ARCHIVE] Archive_GetRomPath OK: %s\n", rom_path);

    return ret;

FAILED:
    ret = -1;
    goto END;
}

/*
typedef struct
{
    char name[256];
    uint32_t crc;
    char *buf;
    size_t size;
} RomCacheThreadArgs;

int WriteRomCahceThread(SceSize args, void *argp)
{
    RomCacheThreadArgs *arg = (RomCacheThreadArgs *)argp;
    char rom_path[MAX_NAME_LENGTH];
    strcpy(rom_path, CORE_CACHE_DIR "/");
    strcat(rom_path, arg->name);
    if (WriteFile(rom_path, arg->buf, arg->size) > 0)
    {
        Archive_InsertRomCache(arg->crc, arg->name);
    }

    free(arg->buf);
    AppLog("[ARCHIVE] AsyncWriteRomCache thread end\n");

    sceKernelExitDeleteThread(0);
    return 0;
}

void Archive_AsyncWriteRomCache(uint32_t crc, const char *rom_name, const char *buf, size_t size)
{
    AppLog("[ARCHIVE] AsyncWriteRomCache start\n");
    int write_thread = sceKernelCreateThread("write_rom_cache_thread", WriteRomCahceThread, 0x10000100 + 20, 0x10000, 0, 0, NULL);
    if (write_thread >= 0)
    {
        RomCacheThreadArgs args;
        strcpy(args.name, rom_name);
        args.crc = crc;
        args.size = size;
        args.buf = malloc(size);
        if (args.buf)
        {
            memcpy(args.buf, buf, size);
            sceKernelStartThread(write_thread, sizeof(args), &args);
        }
    }
}*/