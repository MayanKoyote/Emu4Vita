#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <psp2/io/dirent.h>

#include "wallpaper.h"
#include "strnatcmp.h"

int WallpaperListAdd(WallpaperList *list, WallpaperEntry *entry)
{
    if (!list || !entry)
        return 0;

    entry->next = NULL;
    entry->prev = NULL;

    if (list->head == NULL)
    {
        list->head = entry;
        list->tail = entry;
    }
    else
    {
        WallpaperEntry *insert = list->head;

        while (insert)
        {
            if (strnatcasecmp(entry->name, insert->name) < 0)
                break;
            insert = insert->next;
        }

        if (insert == NULL)
        {
            WallpaperEntry *tail = list->tail;
            tail->next = entry;
            entry->prev = tail;
            list->tail = entry;
        }
        else
        {
            if (insert->prev)
            {
                insert->prev->next = entry;
                entry->prev = insert->prev;
            }
            insert->prev = entry;
            entry->next = insert;
            if (insert == list->head)
                list->head = entry;
        }
    }

    list->length++;

    return 1;
}

int WallpaperListRemove(WallpaperList *list, WallpaperEntry *entry)
{
    if (!list || !entry)
        return 0;

    if (entry->prev)
    {
        entry->prev->next = entry->next;
    }
    else
    {
        list->head = entry->next;
    }

    if (entry->next)
    {
        entry->next->prev = entry->prev;
    }
    else
    {
        list->tail = entry->prev;
    }

    free(entry);

    list->length--;

    if (list->length == 0)
    {
        list->head = NULL;
        list->tail = NULL;
    }

    return 1;
}

WallpaperEntry *WallpaperListFindEntryByNum(WallpaperList *list, int n)
{
    if (!list)
        return NULL;

    WallpaperEntry *entry = list->head;

    while (n > 0 && entry)
    {
        n--;
        entry = entry->next;
    }

    if (n != 0)
        return NULL;

    return entry;
}

WallpaperEntry *WallpaperListFindEntryByName(WallpaperList *list, const char *name)
{
    if (!list || !name)
        return NULL;

    WallpaperEntry *entry = list->head;

    int name_length = strlen(name);

    while (entry)
    {
        if (entry->name && entry->name_length == name_length && strcasecmp(entry->name, name) == 0)
            return entry;

        entry = entry->next;
    }

    return NULL;
}

int WallpaperListGetNumByName(WallpaperList *list, const char *name)
{
    if (!list || !name)
        return -1;

    WallpaperEntry *entry = list->head;

    int name_length = strlen(name);
    int n = 0;

    while (entry)
    {
        if (entry->name && entry->name_length == name_length && strcasecmp(entry->name, name) == 0)
            return n;

        entry = entry->next;
        n++;
    }

    return -1;
}

int WallpaperListGetEntries(WallpaperList *list)
{
    if (!list)
        return -1;

    SceUID dfd = sceIoDopen(list->path);
    if (dfd < 0)
        return dfd;

    int res = 0;

    do
    {
        SceIoDirent dir;
        memset(&dir, 0, sizeof(SceIoDirent));

        res = sceIoDread(dfd, &dir);
        if (res > 0)
        {
            if (SCE_S_ISDIR(dir.d_stat.st_mode))
                continue;
            
            const char *p = strrchr(dir.d_name, '.');
            if (!*p++ || strcasecmp(p, "png") != 0)
                continue;

            WallpaperEntry *entry = (WallpaperEntry *)calloc(1, sizeof(WallpaperEntry));
            if (!entry)
                continue;

            entry->name_length = strlen(dir.d_name);
            entry->name = (char *)malloc(entry->name_length + 1);
            if (entry->name)
                strcpy(entry->name, dir.d_name);

            WallpaperListAdd(list, entry);
        }
    } while (res > 0);

    sceIoDclose(dfd);
    return 0;
}

void WallpaperListEmpty(WallpaperList *list)
{
    if (!list)
        return;

    WallpaperEntry *entry = list->head;

    while (entry)
    {
        WallpaperEntry *next = entry->next;
        free(entry);
        entry = next;
    }

    list->head = NULL;
    list->tail = NULL;
    list->length = 0;
}