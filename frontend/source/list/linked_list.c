#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "linked_list.h"

struct LinkedListEntry
{
    LinkedList *parent;
    struct LinkedListEntry *next;
    struct LinkedListEntry *prev;
    void *data;
    LinkedListFreeDataCallback freeEntryData;
};

struct LinkedList
{
    LinkedListEntry *head;
    LinkedListEntry *tail;
    int length;
    void *data;
    LinkedListFreeDataCallback freeEntryData;
    LinkedListFreeDataCallback freeListData;
    LinkedListCompareCallback compare;
};

void *LinkedListGetEntryData(LinkedListEntry *entry)
{
    return entry ? entry->data : NULL;
}

void LinkedListSetEntryData(LinkedListEntry *entry, void *data)
{
    if (entry)
        entry->data = data;
}

void *LinkedListGetListData(LinkedList *list)
{
    return list ? list->data : NULL;
}

void LinkedListSetListData(LinkedList *list, void *data)
{
    if (list)
        list->data = data;
}

int LinkedListGetLength(LinkedList *list)
{
    return list ? list->length : 0;
}

void LinkedListEntrySetFreeEntryDataCallback(LinkedListEntry *entry, LinkedListFreeDataCallback callback)
{
    if (entry)
        entry->freeEntryData = callback;
}

void LinkedListSetFreeEntryDataCallback(LinkedList *list, LinkedListFreeDataCallback callback)
{
    if (list)
        list->freeEntryData = callback;
}

void LinkedListSetFreeListDataCallback(LinkedList *list, LinkedListFreeDataCallback callback)
{
    if (list)
        list->freeListData = callback;
}

void LinkedListSetCompareCallback(LinkedList *list, LinkedListCompareCallback callback)
{
    if (list)
        list->compare = callback;
}

LinkedListEntry *LinkedListHead(LinkedList *list)
{
    return list ? list->head : NULL;
}

LinkedListEntry *LinkedListTail(LinkedList *list)
{
    return list ? list->tail : NULL;
}

LinkedListEntry *LinkedListNext(LinkedListEntry *entry)
{
    return entry ? entry->next : NULL;
}

LinkedListEntry *LinkedListPrev(LinkedListEntry *entry)
{
    return entry ? entry->prev : NULL;
}

LinkedListEntry *LinkedListFindByNum(LinkedList *list, int n)
{
    if (!list)
        return NULL;

    LinkedListEntry *entry = list->head;

    while (n > 0 && entry)
    {
        n--;
        entry = entry->next;
    }

    if (n != 0)
        return NULL;

    return entry;
}

LinkedListEntry *LinkedListFindByData(LinkedList *list, void *data)
{
    if (!list)
        return NULL;

    LinkedListEntry *entry = list->head;

    while (entry)
    {
        if (entry->data == data)
            return entry;
        entry = entry->next;
    }

    return NULL;
}

int LinkedListRemove(LinkedList *list, LinkedListEntry *entry)
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

    if (entry->data && list->freeEntryData)
        list->freeEntryData(entry->data);
    free(entry);

    list->length--;

    if (list->length == 0)
    {
        list->head = NULL;
        list->tail = NULL;
    }

    return 1;
}

static LinkedListEntry *LinkedListAddBase(LinkedList *list, LinkedListEntry *insert, void *data)
{
    if (!list || !data)
        return NULL;

    if (insert && insert->parent != list)
        return NULL;

    LinkedListEntry *entry = malloc(sizeof(LinkedListEntry));
    if (!entry)
        return NULL;

    entry->next = NULL;
    entry->prev = NULL;
    entry->data = data;
    entry->parent = list;
    entry->freeEntryData = NULL;

    if (list->head == NULL)
    {
        list->head = entry;
        list->tail = entry;
    }
    else
    {
        if (insert == NULL && list->compare)
        {
            insert = list->head;
            while (insert)
            {
                if (list->compare(data, insert->data) < 0)
                    break;
                insert = insert->next;
            }
        }

        if (insert == NULL)
        {
            LinkedListEntry *tail = list->tail;
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

    return entry;
}

LinkedListEntry *LinkedListAdd(LinkedList *list, void *data)
{
    return LinkedListAddBase(list, NULL, data);
}

LinkedListEntry *LinkedListAddAbove(LinkedList *list, LinkedListEntry *above, void *data)
{
    return LinkedListAddBase(list, above, data);
}

LinkedListEntry *LinkedListAddBelow(LinkedList *list, LinkedListEntry *below, void *data)
{
    if (below)
        return LinkedListAddBase(list, below->next, data);
    return LinkedListAddBase(list, NULL, data);
}

void LinkedListEmpty(LinkedList *list)
{
    if (!list)
        return;

    LinkedListEntry *entry = list->head;

    while (entry)
    {
        LinkedListEntry *next = entry->next;
        if (entry->data)
        {
            if (entry->freeEntryData)
                entry->freeEntryData(entry->data);
            else if (list->freeEntryData)
                list->freeEntryData(entry->data);
        }
        free(entry);
        entry = next;
    }

    list->head = NULL;
    list->tail = NULL;
    list->length = 0;
}

void LinkedListDestroy(LinkedList *list)
{
    if (!list)
        return;

    LinkedListEmpty(list);

    if (list->data && list->freeListData)
        list->freeListData(list->data);
    free(list);
}

LinkedList *NewLinkedList()
{
    LinkedList *list = (LinkedList *)calloc(1, sizeof(LinkedList));
    if (!list)
        return NULL;

    return list;
}
