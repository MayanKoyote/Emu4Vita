#ifndef __M_WALLPAPER_H__
#define __M_WALLPAPER_H__

#include "file.h"

typedef struct WallpaperEntry
{
    struct WallpaperEntry *prev;
    struct WallpaperEntry *next;
    char *name;
    int name_length;
} WallpaperEntry;

typedef struct
{
    WallpaperEntry *head;
    WallpaperEntry *tail;
    char path[MAX_PATH_LENGTH];
    int length;
} WallpaperList;

int WallpaperListAdd(WallpaperList *list, WallpaperEntry *entry);
int WallpaperListRemove(WallpaperList *list, WallpaperEntry *entry);
WallpaperEntry *WallpaperListFindEntryByNum(WallpaperList *list, int n);
WallpaperEntry *WallpaperListFindEntryByName(WallpaperList *list, const char *name);
int WallpaperListGetNumByName(WallpaperList *list, const char *name);
int WallpaperListGetEntries(WallpaperList *list);
void WallpaperListEmpty(WallpaperList *list);

#endif
