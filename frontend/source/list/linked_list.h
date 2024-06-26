#ifndef __M_LINKED_LIST_H__
#define __M_LINKED_LIST_H__

typedef void (*LinkedListFreeDataCallback)(void *data);
typedef int (*LinkedListCompareCallback)(void *data1, void *data2);

typedef struct LinkedListEntry LinkedListEntry;
typedef struct LinkedList LinkedList;

void *LinkedListGetEntryData(LinkedListEntry *entry);
void LinkedListSetEntryData(LinkedListEntry *entry, void *data);
void *LinkedListGetListData(LinkedList *list);
void LinkedListSetListData(LinkedList *list, void *data);
int LinkedListGetLength(LinkedList *list);
LinkedList *LinkedListEntryGetParent(LinkedListEntry *entry);
void LinkedListEntrySetFreeEntryDataCallback(LinkedListEntry *entry, LinkedListFreeDataCallback callback);
void LinkedListSetFreeEntryDataCallback(LinkedList *list, LinkedListFreeDataCallback callback);
void LinkedListSetFreeListDataCallback(LinkedList *list, LinkedListFreeDataCallback callback);
void LinkedListSetCompareCallback(LinkedList *list, LinkedListCompareCallback callback);

LinkedListEntry *LinkedListHead(LinkedList *list);
LinkedListEntry *LinkedListTail(LinkedList *list);
LinkedListEntry *LinkedListNext(LinkedListEntry *entry);
LinkedListEntry *LinkedListPrev(LinkedListEntry *entry);
LinkedListEntry *LinkedListFindByNum(LinkedList *list, int n);
LinkedListEntry *LinkedListFindByData(LinkedList *list, void *data);
int LinkedListRemove(LinkedList *list, LinkedListEntry *entry);
LinkedListEntry *LinkedListAdd(LinkedList *list, void *data);
LinkedListEntry *LinkedListAddAbove(LinkedList *list, LinkedListEntry *above, void *data);
LinkedListEntry *LinkedListAddBelow(LinkedList *list, LinkedListEntry *below, void *data);
void LinkedListEmpty(LinkedList *list);
void LinkedListDestroy(LinkedList *list);
LinkedList *NewLinkedList();

#endif