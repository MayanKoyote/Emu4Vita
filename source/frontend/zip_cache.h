#ifndef __M_ZIP_CACHE_H__
#define __M_ZIP_CACHE_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    void InitZipCache();
    const char *GetZipCacheRom(const char *name);

#ifdef __cplusplus
}
#endif

#endif