#ifndef __M_LIST_VIEW_H__
#define __M_LIST_VIEW_H__

#include "list/linked_list.h"
#include "Layout.h"
#include "TextView.h"

enum TypeListViewPosDisplayMode
{
    TYPE_LISTVIEW_FOCUS_POS_DISPLAY_MODE_DEFAULT,         // 默认动态滚动
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

typedef struct ListViewCallbacks
{
    int (*onGetListLength)(void *list);
    void *(*onGetHeadListEntry)(void *list);
    void *(*onGetNextListEntry)(void *list, void *entry, int id);

    void *(*onCreateItemView)(void *data);
    int (*onSetItemViewData)(void *itemView, void *data);
    int (*onBeforeDrawItemView)(ListView *listView, void *itemView, int id);
    int (*onItemClick)(ListView *listView, void *itemView, int id);
    int (*onItemLongClick)(ListView *listView, void *itemView, int id);
} ListViewCallbacks;

void *ListViewFindItemByNum(ListView *listView, int n);
int ListViewAddItemByData(ListView *listView, void *data);
int ListViewRemoveItemByNum(ListView *listView, int n);
int ListViewEmpty(ListView *listView);
int ListViewRefreshtList(ListView *listView);

int ListViewSetBgColor(ListView *listView, uint32_t color);
int ListViewSetData(ListView *listView, void *data);
int ListViewSetList(ListView *listView, void *list, ListViewCallbacks *callbacks);
int ListViewSetCurrentScrollY(ListView *listView, int y);
int ListViewSetTargetScrollY(ListView *listView, int y);
int ListViewSetTopPos(ListView *listView, int pos);
int ListViewSetFocusPos(ListView *listView, int pos);

void *ListViewGetData(ListView *listView);
void *ListViewGetList(ListView *listView);
int ListViewGetCurrentScrollY(ListView *listView);
int ListViewGetTargetScrollY(ListView *listView);
int ListViewGetTopPos(ListView *listView);
int ListViewGetFocusPos(ListView *listView);

int ListViewMoveTopPos(ListView *listView, int move_type);
int ListViewMoveFocusPos(ListView *listView, int move_type);

int ListViewSetDividerSize(ListView *listView, int size);
int ListViewSetDividerColor(ListView *listView, uint32_t color);

ListView *NewListView();

#endif
