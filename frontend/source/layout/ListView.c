#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "list/linked_list.h"
#include "gui/gui.h"
#include "Layout.h"
#include "utils.h"

struct ListView
{
    LayoutParams params;
    uint32_t bg_color;
    LinkedList *items;
    void *list;
    int choose_mode;
    int focus_pos;
    int driver_size;
    uint32_t driver_color;
    uint32_t focus_color;
    int current_scroll_x;
    int current_scroll_y;
    int target_scroll_x;
    int target_scroll_y;
    int scroll_step;
    ListViewCallbacks callbacks;
};

static void ListViewUpdateScroll(ListView *listView)
{
    if (listView->current_scroll_y < listView->target_scroll_y)
    {
        listView->current_scroll_y += listView->scroll_step;
        if (listView->current_scroll_y > listView->target_scroll_y)
            listView->current_scroll_y = listView->target_scroll_y;
    }
    else if (listView->current_scroll_y > listView->target_scroll_y)
    {
        listView->current_scroll_y -= listView->scroll_step;
        if (listView->current_scroll_y < listView->target_scroll_y)
            listView->current_scroll_y = listView->target_scroll_y;
    }
}

static void ListViewDestroy(void *view)
{
    ListView *listView = (ListView *)view;
    LayoutParams *params = LayoutParamsGetParams(listView);

    if (!listView || params->dont_free)
        return;

    LinkedListDestroy(listView->items);
    free(listView);
}

static int ListViewUpdateChildOne(ListView *listView, void *itemView, int available_w, int available_h, int *wrap_w, int *wrap_h, int has_driver)
{
    if (!listView || !itemView)
        return -1;

    LayoutParams *ls_params = LayoutParamsGetParams(listView);
    LayoutParams *it_params = LayoutParamsGetParams(itemView);

    LayoutParamsSetAvailableSize(itemView, available_w, available_h);
    LayoutParamsUpdate(itemView);

    int occupy_w = it_params->measured_w + it_params->margin_left + it_params->margin_right;
    int occupy_h = it_params->measured_h + it_params->margin_top + it_params->margin_bottom;

    if (ls_params->orientation == TYPE_LAYOUT_ORIENTATION_HORIZONTAL)
    {
        *wrap_w += occupy_w;
        if (has_driver)
            *wrap_w += listView->driver_size;
        if (occupy_h > *wrap_h)
            *wrap_h = occupy_h;
    }
    else if (ls_params->orientation == TYPE_LAYOUT_ORIENTATION_VERTICAL)
    {
        *wrap_h += occupy_h;
        if (has_driver)
            *wrap_h += listView->driver_size;
        if (occupy_w > *wrap_w)
            *wrap_w = occupy_w;
    }

    return 0;
}

static int ListViewUpdate(void *view)
{
    if (!view)
        return -1;

    ListView *listView = (ListView *)view;
    LayoutParams *params = LayoutParamsGetParams(listView);

    if (params->available_w <= 0 || params->available_h <= 0)
    {
        params->measured_w = 0;
        params->measured_h = 0;
        return -1;
    }

    int view_max_w = params->available_w - params->margin_left - params->margin_right;
    int view_max_h = params->available_h - params->margin_top - params->margin_bottom;
    int view_available_w = view_max_w - params->padding_left - params->padding_right;
    int view_available_h = view_max_h - params->padding_top - params->padding_bottom;
    int view_wrap_w = 0;
    int view_wrap_h = 0;

    if (params->orientation == TYPE_LAYOUT_ORIENTATION_HORIZONTAL)
        view_available_w = MAX_AVAILABLE_WIDTH;
    else if (params->orientation == TYPE_LAYOUT_ORIENTATION_VERTICAL)
        view_available_h = MAX_AVAILABLE_HEIGHT;

    LinkedListEntry *entry = LinkedListHead(listView->items);
    while (entry)
    {
        LinkedListEntry *next = LinkedListNext(entry);
        void *itemView = LinkedListGetEntryData(entry);
        int has_driver = (next != NULL); // 不绘制最后一个driver
        ListViewUpdateChildOne(listView, itemView, view_available_w, view_available_h, &view_wrap_w, &view_wrap_h, has_driver);
        entry = next;
    }

    listView->current_scroll_x = listView->target_scroll_x = 0;
    listView->current_scroll_y = listView->target_scroll_y = 0;

    view_wrap_w += (params->padding_left + params->padding_right);
    view_wrap_h += (params->padding_top + params->padding_bottom);

    params->wrap_w = view_wrap_w;
    params->wrap_h = view_wrap_h;

    if (params->layout_w == TYPE_LAYOUT_MATH_PARENT)
        params->measured_w = view_max_w;
    else if (params->layout_w == TYPE_LAYOUT_WRAP_CONTENT)
        params->measured_w = view_wrap_w;
    else
        params->measured_w = params->layout_w;
    if (params->measured_w > view_max_w)
        params->measured_w = view_max_w;
    if (params->measured_w < 0)
        params->measured_w = 0;

    if (params->layout_h == TYPE_LAYOUT_MATH_PARENT)
        params->measured_h = view_max_h;
    else if (params->layout_h == TYPE_LAYOUT_WRAP_CONTENT)
        params->measured_h = view_wrap_h;
    else
        params->measured_h = params->layout_h;
    if (params->measured_h > view_max_h)
        params->measured_h = view_max_h;
    if (params->measured_h < 0)
        params->measured_h = 0;

    return 0;
}

static int ListViewDrawChildOne(ListView *listView, void *itemView, int *x, int *y, int min_x, int min_y, int max_x, int max_y, int index, int has_driver)
{
    if (!listView || !itemView)
        return 0;

    LayoutParams *ls_params = LayoutParamsGetParams(listView);
    LayoutParams *it_params = LayoutParamsGetParams(itemView);

    int occupy_w = it_params->measured_w + it_params->margin_left + it_params->margin_right;
    int occupy_h = it_params->measured_h + it_params->margin_top + it_params->margin_bottom;
    int dx = *x + occupy_w;
    int dy = *y + occupy_h;
    int next_x = *x;
    int next_y = *y;

    if (ls_params->orientation == TYPE_LAYOUT_ORIENTATION_HORIZONTAL)
    {
        next_x = dx;
        if (has_driver)
            next_x += listView->driver_size;
        if (dx <= min_x)
            goto NEXT;
        if (*x >= max_x)
            return -1;
    }
    else if (ls_params->orientation == TYPE_LAYOUT_ORIENTATION_VERTICAL)
    {
        next_y = dy;
        if (has_driver)
            next_y += listView->driver_size;
        if (dy <= min_y)
            goto NEXT;
        if (*y >= max_y)
            return -1;
    }

    LayoutParamsSetLayoutPosition(itemView, *x, *y);
    LayoutParamsDraw(itemView);

    if (has_driver && listView->driver_color && listView->driver_size > 0)
    {
        if (ls_params->orientation == TYPE_LAYOUT_ORIENTATION_HORIZONTAL)
            GUI_DrawFillRectangle(dx, *y, listView->driver_size, it_params->measured_h, listView->driver_color);
        else if (ls_params->orientation == TYPE_LAYOUT_ORIENTATION_VERTICAL)
            GUI_DrawFillRectangle(*x, dy, it_params->measured_w, listView->driver_size, listView->driver_color);
    }

NEXT:
    *x = next_x;
    *y = next_y;

    return 0;
}

void ListViewDraw(void *view)
{
    if (!view)
        return;

    ListView *listView = (ListView *)view;
    LayoutParams *params = LayoutParamsGetParams(listView);

    if (params->measured_w <= 0 || params->measured_h <= 0)
        return;

    int view_x = params->layout_x + params->margin_left;
    int view_y = params->layout_y + params->margin_top;
    int view_max_w = params->measured_w;
    int view_max_h = params->measured_h;

    int child_sx = view_x + params->padding_left;
    int child_sy = view_y + params->padding_top;
    int child_dx = view_x + view_max_w - params->padding_right;
    int child_dy = view_y + view_max_h - params->padding_bottom;
    int child_max_w = child_dx - child_sx;
    int child_max_h = child_dy - child_sy;

    int x = child_sx + listView->current_scroll_x;
    int y = child_sy + listView->current_scroll_y;

    if (listView->bg_color)
        GUI_DrawFillRectangle(view_x, view_y, view_max_w, view_max_h, listView->bg_color);

    if (listView->items)
    {
        ListViewUpdateScroll(listView); // 更新滚动数据

        GUI_SetClipping(child_sx, child_sy, child_max_w, child_max_h);

        int index = 0;
        LinkedListEntry *entry = LinkedListHead(listView->items);
        while (entry)
        {
            LinkedListEntry *next = LinkedListNext(entry);
            void *itemView = LinkedListGetEntryData(entry);
            int has_driver = (next != NULL); // 不绘制最后一个driver

            if (listView->callbacks.updateDrawItemView)
                listView->callbacks.updateDrawItemView(listView, itemView, index);

            if (ListViewDrawChildOne(listView, itemView, &x, &y, child_sx, child_sy, child_dx, child_dy, index, has_driver) < 0)
                break;
            entry = next;
            index++;
        }

        GUI_UnsetClipping();

        // Draw scrollbar
        int scrollbar_track_x = view_x + view_max_w - GUI_DEF_SCROLLBAR_SIZE;
        int scrollbar_track_y = view_y;
        int scrollbar_track_height = view_max_h;
        LayoutParams *ls_params = LayoutParamsGetParams(listView);
        int its_measured_h = ls_params->measured_h - ls_params->padding_top - ls_params->padding_bottom;
        int its_wrap_h = ls_params->wrap_h - ls_params->padding_top - ls_params->padding_bottom;
        GUI_DrawVerticalScrollbar(scrollbar_track_x, scrollbar_track_y, scrollbar_track_height,
                                  its_wrap_h, its_measured_h, 0 - listView->current_scroll_y, 0);
    }
}

void *ListViewFindItemByNum(ListView *listView, int n)
{
    if (!listView || !listView->items)
        return 0;

    LinkedListEntry *entry = LinkedListFindByNum(listView->items, n);
    return LinkedListGetEntryData(entry);
}

int ListViewAddItemByData(ListView *listView, void *data)
{
    if (!listView || !listView->items)
        return 0;

    ListViewCallbacks *callbacks = &listView->callbacks;
    if (!callbacks->newItemView || !callbacks->setItemViewData)
        return 0;

    void *itemView = callbacks->newItemView(data);
    if (!itemView)
        return 0;

    callbacks->setItemViewData(itemView, data);
    LinkedListAdd(listView->items, itemView);

    return 1;
}

int ListViewRemoveItemByNum(ListView *listView, int n)
{
    if (!listView || !listView->items)
        return 0;

    LinkedListEntry *entry = LinkedListFindByNum(listView->items, n);
    if (entry)
    {
        LinkedListRemove(listView->items, entry);
        return 1;
    }

    return 0;
}

int ListViewEmpty(ListView *listView)
{
    if (!listView)
        return -1;

    LinkedListEmpty(listView->items);

    return 0;
}

int ListViewRefreshtList(ListView *listView)
{
    if (!listView || !listView->items)
        return -1;

    ListViewEmpty(listView);

    if (!listView->list)
        return -1;

    ListViewCallbacks *callbacks = &listView->callbacks;
    if (!callbacks->getHeadListEntry || !callbacks->getNextListEntry)
        return -1;

    void *entry = callbacks->getHeadListEntry(listView->list);
    int n = 0;
    while (entry)
    {
        ListViewAddItemByData(listView, entry);
        entry = callbacks->getNextListEntry(listView->list, entry, n);
        n++;
    }

    return 0;
}

int ListViewSetList(ListView *listView, void *list, ListViewCallbacks *callbacks)
{
    if (!listView)
        return -1;

    listView->list = list;
    if (callbacks)
        memcpy(&listView->callbacks, callbacks, sizeof(ListViewCallbacks));
    else
        memset(&listView->callbacks, 0, sizeof(ListViewCallbacks));

    ListViewRefreshtList(listView);

    return 0;
}

int ListViewGetSrcollYByPos(ListView *listView, int pos)
{
    if (!listView || !listView->items)
        return 0;

    int y = 0;

    LinkedListEntry *entry = LinkedListHead(listView->items);
    int i;
    for (i = 0; i < pos && entry; i++)
    {
        void *itemView = LinkedListGetEntryData(entry);
        LayoutParams *it_params = LayoutParamsGetParams(itemView);
        int occupy_h = it_params->measured_h + it_params->margin_top + it_params->margin_bottom + listView->driver_size;
        y -= occupy_h;
        entry = LinkedListNext(entry);
    }

    return y;
}

int ListViewGetPosBySrcollY(ListView *listView, int scroll_y)
{
    if (!listView || !listView->items)
        return 0;

    int pos = 0;
    int y = 0;

    LayoutParams *ls_params = LayoutParamsGetParams(listView);
    int its_wrap_h = ls_params->wrap_h - ls_params->padding_top - ls_params->padding_bottom;
    int max_scroll_y = 0;
    int min_scroll_y = 0 - its_wrap_h;

    if (scroll_y < min_scroll_y)
        scroll_y = min_scroll_y;
    if (scroll_y > max_scroll_y)
        scroll_y = max_scroll_y;

    LinkedListEntry *entry = LinkedListHead(listView->items);
    while (entry)
    {
        void *itemView = LinkedListGetEntryData(entry);
        LayoutParams *it_params = LayoutParamsGetParams(itemView);
        int occupy_h = it_params->measured_h + it_params->margin_top + it_params->margin_bottom + listView->driver_size;
        int dy = y + occupy_h;
        if (y <= scroll_y && dy > scroll_y)
            return pos;
        y -= occupy_h;
        pos++;
        entry = LinkedListNext(entry);
    }

    return pos;
}

int ListViewSetBgColor(ListView *listView, uint32_t color)
{
    if (!listView)
        return -1;

    listView->bg_color = color;
    return 0;
}

int ListViewSetTargetScrollY(ListView *listView, int y)
{
    if (!listView)
        return -1;

    LayoutParams *ls_params = LayoutParamsGetParams(listView);
    int its_measured_h = ls_params->measured_h - ls_params->padding_top - ls_params->padding_bottom;
    int its_wrap_h = ls_params->wrap_h - ls_params->padding_top - ls_params->padding_bottom;
    int max_scroll_y = 0;
    int min_scroll_y = its_wrap_h > its_measured_h ? (0 - its_wrap_h + its_measured_h) : 0;

    if (y < min_scroll_y)
        y = min_scroll_y;
    if (y > max_scroll_y)
        y = max_scroll_y;

    if (listView->target_scroll_y != y)
    {
        listView->target_scroll_y = y;
        listView->scroll_step = (listView->target_scroll_y - listView->current_scroll_y) / 10;
        if (listView->scroll_step < 0)
            listView->scroll_step = 0 - listView->scroll_step;
        if (listView->scroll_step < 1)
            listView->scroll_step = 1;
    }

    // 自动滚动有bug，画面会有闪烁，且销毁Texture里画面会有撕裂，暂时设置current=target关闭自动滚动，可注释掉这行测试自动滚动功能
    listView->current_scroll_y = listView->target_scroll_y;

    return 0;
}

int ListViewSetTopPos(ListView *listView, int pos)
{
    if (!listView)
        return -1;

    int y = 0;
    int length = LinkedListGetLength(listView->items);

    if (pos > length - 1)
        pos = length - 1;
    if (pos < 0)
        pos = 0;

    if (pos > 0)
        y = ListViewGetSrcollYByPos(listView, pos);
    ListViewSetTargetScrollY(listView, y);

    return 0;
}

int ListViewSetFocusPos(ListView *listView, int pos)
{
    if (!listView || !listView->items)
        return -1;

    int y = 0;
    int length = LinkedListGetLength(listView->items);

    if (pos > length - 1)
        pos = length - 1;
    if (pos < 0)
        pos = 0;
    listView->focus_pos = pos;

    if (pos > 0)
    {
        void *itemView = ListViewFindItemByNum(listView, pos);
        if (itemView)
        {
            LayoutParams *ls_params = LayoutParamsGetParams(listView);
            LayoutParams *it_params = LayoutParamsGetParams(itemView);
            int its_measured_h = ls_params->measured_h - ls_params->padding_top - ls_params->padding_bottom;
            int pos_occupy_h = it_params->measured_h + it_params->margin_top + it_params->margin_bottom;
            y = ListViewGetSrcollYByPos(listView, pos) + (its_measured_h / 2 - pos_occupy_h / 2); // 设置focus居中
        }
    }

    ListViewSetTargetScrollY(listView, y);

    return 0;
}

int ListViewGetTargetScrollY(ListView *listView)
{
    return listView ? listView->target_scroll_y : 0;
}

int ListViewGetTopPos(ListView *listView)
{
    return listView ? ListViewGetPosBySrcollY(listView, listView->target_scroll_y) : -1;
}

int ListViewGetFocusPos(ListView *listView)
{
    return listView ? listView->focus_pos : -1;
}

int ListViewMoveTopPos(ListView *listView, int move_type)
{
    if (!listView || !listView->items)
        return -1;

    LayoutParams *ls_params = LayoutParamsGetParams(listView);
    int its_measured_h = ls_params->measured_h - ls_params->padding_top - ls_params->padding_bottom;
    int y = listView->target_scroll_y;

    if (move_type == TYPE_MOVE_UP)
    {
        int top_pos = ListViewGetPosBySrcollY(listView, listView->target_scroll_y);
        y = ListViewGetSrcollYByPos(listView, top_pos - 1);
    }
    else if (move_type == TYPE_MOVE_DOWN)
    {
        int top_pos = ListViewGetPosBySrcollY(listView, listView->target_scroll_y);
        y = ListViewGetSrcollYByPos(listView, top_pos + 1);
    }
    else if (move_type == TYPE_MOVE_LEFT)
    {
        y = listView->target_scroll_y + its_measured_h;
    }
    else if (move_type == TYPE_MOVE_RIGHT)
    {
        y = listView->target_scroll_y - its_measured_h;
    }

    ListViewSetTargetScrollY(listView, y);

    return 0;
}

int ListViewMoveFocusPos(ListView *listView, int move_type)
{
    if (!listView || !listView->items)
        return -1;

    int focus_pos = listView->focus_pos;
    LayoutParams *ls_params = LayoutParamsGetParams(listView);
    int its_measured_h = ls_params->measured_h - ls_params->padding_top - ls_params->padding_bottom;

    if (move_type == TYPE_MOVE_UP)
    {
        if (focus_pos > 0)
            focus_pos--;
    }
    else if (move_type == TYPE_MOVE_DOWN)
    {
        if (focus_pos < LinkedListGetLength(listView->items) - 1)
            focus_pos++;
    }
    else if (move_type == TYPE_MOVE_LEFT)
    {
        int y = ListViewGetSrcollYByPos(listView, focus_pos) + its_measured_h;
        focus_pos = ListViewGetPosBySrcollY(listView, y);
    }
    else if (move_type == TYPE_MOVE_RIGHT)
    {
        int y = ListViewGetSrcollYByPos(listView, focus_pos) - its_measured_h;
        focus_pos = ListViewGetPosBySrcollY(listView, y);
    }

    if (focus_pos != listView->focus_pos)
        ListViewSetFocusPos(listView, focus_pos);

    return 0;
}

int ListViewSetDriverSize(ListView *listView, int size)
{
    if (!listView)
        return -1;

    listView->driver_size = size;

    return 0;
}

int ListViewSetDriverColor(ListView *listView, uint32_t color)
{
    if (!listView)
        return -1;

    listView->driver_color = color;

    return 0;
}

int ListViewInit(ListView *listView)
{
    if (!listView)
        return -1;

    memset(listView, 0, sizeof(ListView));

    listView->items = NewLinkedList();
    if (!listView->items)
        return -1;
    LinkedListSetFreeEntryDataCallback(listView->items, LayoutParamsDestroy);

    LayoutParams *params = LayoutParamsGetParams(listView);
    params->destroy = ListViewDestroy;
    params->update = ListViewUpdate;
    params->draw = ListViewDraw;

    return 0;
}

ListView *NewListView()
{
    ListView *listView = (ListView *)malloc(sizeof(ListView));
    if (!listView)
        return NULL;

    if (ListViewInit(listView) < 0)
    {
        free(listView);
        return NULL;
    }

    return listView;
}
