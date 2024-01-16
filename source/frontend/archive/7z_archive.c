#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include <psp2/io/fcntl.h>

#include "config.h"
#include "file.h"

int SevenZ_OpenRom(const char *archive_path, uint32_t *crc, char *name)
{
    return -1;
}

void SevenZ_CloseRom()
{
}

int SevenZ_ExtractRomMemory(void **buf, size_t *size)
{
    return -1;
}

int SevenZ_ExtractRom(const char *rom_name, char *rom_path)
{
    return -1;
}
