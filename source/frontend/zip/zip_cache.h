#ifndef __M_ZIP_CACHE_H__
#define __M_ZIP_CACHE_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif
    // 初始化 zipcache，创建CORE_ZIPCACHE_DIR文件夹，或者从 ZIP_CACHE_CONFIG_PATH 读取 cache 信息。
    void InitZipCache();
    // 传入zip文件路径，返回实际的 rom 路径。
    const char *GetZipCacheRomPath(const char *name);
    // 传入zip文件路径和rom data指针，返回rom的大小，rom指针指向解压的rom内存。
    // rom 使用完后，需要用 free 释放。
    int64_t GetZipCacheRomMemory(const char *name, void **rom);

#ifdef __cplusplus
}
#endif

#endif