#ifndef __M_LIST_VIEW_H__
#define __M_LIST_VIEW_H__

#include "list/linked_list.h"
#include "Layout.h"
#include "TextView.h"

enum TypeListViewPosDisplayMode
{
    TYPE_LISTVIEW_FOCUS_POS_DISPLAY_MODE_DEFAULT,
    TYPE_LISTVIEW_FOCUS_POS_DISPLAY_MODE_CENTER,          // 动态居中
    TYPE_LISTVIEW_FOCUS_POS_DISPLAY_MODE_CENTER_ABSOLUTE, // 绝对居中
};

enum TypeListViewChooseMode
{
    TYPE_LISTVIEW_CHOOSE_MODE_DISABLE,
    TYPE_LISTVIEW_CHOOSE_MODE_SINGLE,
    TYPE_LISTVIEW_CHOOSE_MODE_MULTI,
};

typedef struct ListView ListView;

typedef struct
{
    void *(*newItemView)(void *data);
    int (*setItemViewData)(void *itemView, void *data);
    int (*updateDrawItemView)(ListView *listView, void *itemView, int n);

    int (*getListLength)(void *list);
    void *(*getHeadListEntry)(void *list);
    void *(*getNextListEntry)(void *list, void *cur_entry, int cur_idx);


    char *(*onItemClick)(void *list, int n);
    char *(*onItemLongClick)(void *list, int n);
} ListViewCallbacks;

void *ListViewFindItemByNum(ListView *listView, int n);
int ListViewAddItemByData(ListView *listView, void *data);
int ListViewRemoveItemByNum(ListView *listView, int n);
int ListViewEmpty(ListView *listView);
int ListViewRefreshtList(ListView *listView);
int ListViewSetList(ListView *listView, void *list, ListViewCallbacks *callbacks);

int ListViewSetBgColor(ListView *listView, uint32_t color);
int ListViewSetTargetScrollY(ListView *listView, int y);
int ListViewSetTopPos(ListView *listView, int pos);
int ListViewSetFocusPos(ListView *listView, int pos);

int ListViewGetTargetScrollY(ListView *listView);
int ListViewGetTopPos(ListView *listView);
int ListViewGetFocusPos(ListView *listView);

int ListViewMoveTopPos(ListView *listView, int move_type);
int ListViewMoveFocusPos(ListView *listView, int move_type);

int ListViewSetDriverSize(ListView *listView, int size);
int ListViewSetDriverColor(ListView *listView, uint32_t color);

int ListViewInit(ListView *listView);
ListView *NewListView();

#endif
