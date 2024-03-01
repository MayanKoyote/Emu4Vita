#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "list/linked_list.h"
#include "gui/gui.h"
#include "Layout.h"

struct Layout
{
    LayoutParams params;
    uint32_t bg_color;
    LinkedList *childs;
};

static LinkedListEntry *LayoutFindListEntryByView(Layout *layout, void *view)
{
    if (!layout || !layout->childs)
        return NULL;

    LinkedListEntry *entry = LinkedListHead(layout->childs);
    while (entry)
    {
        void *find = LinkedListGetEntryData(entry);
        if (find == view)
            return entry;
    }

    return NULL;
}

static void LayoutDestroy(void *view)
{
    Layout *layout = (Layout *)view;
    LayoutParams *params = LayoutParamsGetParams(layout);

    if (!layout || params->dont_free)
        return;

    LinkedListDestroy(layout->childs);
    free(layout);
}

static int LayoutUpdateChildOne(Layout *layout, void *view, int *available_w, int *available_h, int *wrap_w, int *wrap_h)
{
    if (!layout || !view)
        return -1;

    LayoutParams *l_params = LayoutParamsGetParams(layout);
    LayoutParams *c_params = LayoutParamsGetParams(view);

    LayoutParamsSetAvailableSize(view, *available_w, *available_h);
    LayoutParamsUpdate(view);

    int occupy_w = c_params->measured_w + c_params->margin_left + c_params->margin_right;
    int occupy_h = c_params->measured_h + c_params->margin_top + c_params->margin_bottom;

    if (l_params->orientation == TYPE_LAYOUT_ORIENTATION_HORIZONTAL)
    {
        *available_w -= occupy_w;
        *wrap_w += occupy_w;
        if (occupy_h > *wrap_h)
            *wrap_h = occupy_h;
    }
    else if (l_params->orientation == TYPE_LAYOUT_ORIENTATION_VERTICAL)
    {
        *available_h -= occupy_h;
        *wrap_h += occupy_h;
        if (occupy_w > *wrap_w)
            *wrap_w = occupy_w;
    }

    return 0;
}

static int LayoutUpdate(void *view)
{
    if (!view)
        return -1;

    Layout *layout = (Layout *)view;
    LayoutParams *params = LayoutParamsGetParams(layout);

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

    int n_remainings = LinkedListGetLength(layout->childs);
    LinkedListEntry *entry = LinkedListHead(layout->childs);

    // 先更新非MATH_PARENT的子控件，不然MATH_PARENT会占掉所有的空间
    while (entry)
    {
        void *c_view = LinkedListGetEntryData(entry);
        LayoutParams *c_params = LayoutParamsGetParams(c_view);
        if ((params->orientation == TYPE_LAYOUT_ORIENTATION_HORIZONTAL && c_params->layout_w != TYPE_LAYOUT_MATH_PARENT) ||
            (params->orientation == TYPE_LAYOUT_ORIENTATION_VERTICAL && c_params->layout_h != TYPE_LAYOUT_MATH_PARENT))
        {
            LayoutUpdateChildOne(layout, c_view, &view_available_w, &view_available_h, &view_wrap_w, &view_wrap_h);
            n_remainings--;
        }
        entry = LinkedListNext(entry);
    }

    // 再更新MATH_PARENT的子控件，第一个MATH_PARENT会占用掉所有空间
    if (n_remainings > 0)
    {
        entry = LinkedListHead(layout->childs);
        while (entry)
        {
            void *c_view = LinkedListGetEntryData(entry);
            LayoutParams *c_params = LayoutParamsGetParams(c_view);
            if ((params->orientation == TYPE_LAYOUT_ORIENTATION_HORIZONTAL && c_params->layout_w == TYPE_LAYOUT_MATH_PARENT) ||
                (params->orientation == TYPE_LAYOUT_ORIENTATION_VERTICAL && c_params->layout_h == TYPE_LAYOUT_MATH_PARENT))
            {
                LayoutUpdateChildOne(layout, c_view, &view_available_w, &view_available_h, &view_wrap_w, &view_wrap_h);
            }
            entry = LinkedListNext(entry);
        }
    }

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

static int LayoutDrawChildOne(Layout *layout, void *view, int *x, int *y, int min_x, int min_y, int max_x, int max_y)
{
    if (!layout || !view)
        return 0;

    LayoutParams *l_params = LayoutParamsGetParams(layout);
    LayoutParams *c_params = LayoutParamsGetParams(view);

    int occupy_w = c_params->measured_w + c_params->margin_left + c_params->margin_right;
    int occupy_h = c_params->measured_h + c_params->margin_top + c_params->margin_bottom;
    int dx = *x + occupy_w;
    int dy = *y + occupy_h;
    int next_x = *x;
    int next_y = *y;

    if (l_params->orientation == TYPE_LAYOUT_ORIENTATION_HORIZONTAL)
    {
        next_x = dx;
        if (dx <= min_x)
            goto NEXT;
        if (*x >= max_x)
            return -1;
    }
    else if (l_params->orientation == TYPE_LAYOUT_ORIENTATION_VERTICAL)
    {
        next_y = dy;
        if (dy <= min_y)
            goto NEXT;
        if (*y >= max_y)
            return -1;
    }

    LayoutParamsSetLayoutPosition(view, *x, *y);
    LayoutParamsDraw(view);

NEXT:
    *x = next_x;
    *y = next_y;

    return 0;
}

static void LayoutDraw(void *view)
{
    if (!view)
        return;

    Layout *layout = (Layout *)view;
    LayoutParams *params = LayoutParamsGetParams(layout);

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

    int x = child_sx;
    int y = child_sy;

    if (layout->bg_color)
        GUI_DrawFillRectangle(view_x, view_y, view_max_w, view_max_h, layout->bg_color);

    GUI_SetClipping(child_sx, child_sy, child_max_w, child_max_h);

    LinkedListEntry *entry = LinkedListHead(layout->childs);
    while (entry)
    {
        void *view = LinkedListGetEntryData(entry);
        if (LayoutDrawChildOne(layout, view, &x, &y, child_sx, child_sy, child_dx, child_dy) < 0)
            break;
        entry = LinkedListNext(entry);
    }

    GUI_UnsetClipping();
}

int LayoutSetBgColor(Layout *layout, uint32_t color)
{
    if (!layout)
        return -1;

    layout->bg_color = color;
    return 0;
}

int LayoutAddView(Layout *layout, void *view)
{
    if (!layout || !view)
        return 0;

    if (layout->childs)
    {
        LinkedListAdd(layout->childs, view);
        return 1;
    }

    return 0;
}

int LayoutAddViewAbove(Layout *layout, void *view, void *above)
{
    if (!layout || !view || !above)
        return 0;

    LinkedListEntry *a_entry = LayoutFindListEntryByView(layout, above);
    if (a_entry)
    {
        LinkedListAddAbove(layout->childs, a_entry, view);
        return 1;
    }

    return 0;
}

int LayoutAddViewBelow(Layout *layout, void *view, void *below)
{
    if (!layout || !view || !below)
        return 0;

    LinkedListEntry *b_entry = LayoutFindListEntryByView(layout, below);
    if (b_entry)
    {
        LinkedListAddBelow(layout->childs, b_entry, view);
        return 1;
    }

    return 0;
}

int LayoutRemoveView(Layout *layout, void *view)
{
    if (!layout || !view)
        return 0;

    LinkedListEntry *entry = LayoutFindListEntryByView(layout, view);
    if (entry)
    {
        LinkedListRemove(layout->childs, entry);
        return 1;
    }

    return 0;
}

int LayoutEmpty(Layout *layout)
{
    if (!layout)
        return -1;

    LinkedListEmpty(layout->childs);

    return 0;
}

int LayoutInit(Layout *layout)
{
    if (!layout)
        return -1;

    memset(layout, 0, sizeof(Layout));

    layout->childs = NewLinkedList();
    if (!layout->childs)
        return -1;
    LinkedListSetFreeEntryDataCallback(layout->childs, LayoutParamsDestroy);

    LayoutParams *params = LayoutParamsGetParams(layout);
    params->destroy = LayoutDestroy;
    params->update = LayoutUpdate;
    params->draw = LayoutDraw;

    return 0;
}

Layout *NewLayout()
{
    Layout *layout = (Layout *)malloc(sizeof(Layout));
    if (!layout)
        return NULL;

    if (LayoutInit(layout) < 0)
    {
        free(layout);
        return NULL;
    }

    return layout;
}
