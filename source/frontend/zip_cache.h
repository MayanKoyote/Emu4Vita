#ifndef __M_ZIP_CACHE_H__
#define __M_ZIP_CACHE_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    void InitZipCache();
    const char *GetZipCacheRomPath(const char *name);
    int64_t GetZipCacheRomMemory(const char *name, void **rom);

#ifdef __cplusplus
}
#endif

#endif