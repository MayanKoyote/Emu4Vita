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
    LinkedList *children;
    void *userdata;
};

static void LayoutDestroy(void *view)
{
    Layout *layout = (Layout *)view;
    LayoutParams *params = LayoutParamsGetParams(layout);

    if (!layout || params->dont_free)
        return;

    LinkedListDestroy(layout->children);
    free(layout);
}

static int LayoutUpdateChild(Layout *layout, void *view, int *available_w, int *available_h, int *wrap_w, int *wrap_h)
{
    if (!layout || !view)
        return -1;

    LayoutParams *l_params = LayoutParamsGetParams(layout);
    LayoutParams *c_params = LayoutParamsGetParams(view);

    LayoutParamsSetAvailableSize(view, *available_w, *available_h);
    LayoutParamsUpdate(view);

    int occupy_w = c_params->measured_w + c_params->margin_left + c_params->margin_right;
    int occupy_h = c_params->measured_h + c_params->margin_top + c_params->margin_bottom;

    if (l_params->orientation == TYPE_LAYOUT_PARAMS_ORIENTATION_HORIZONTAL)
    {
        *available_w -= occupy_w;
        *wrap_w += occupy_w;
        if (occupy_h > *wrap_h)
            *wrap_h = occupy_h;
    }
    else if (l_params->orientation == TYPE_LAYOUT_PARAMS_ORIENTATION_VERTICAL)
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

    int layout_available_w = params->available_w - params->margin_left - params->margin_right;
    int layout_available_h = params->available_h - params->margin_top - params->margin_bottom;
    int layout_wrap_w = 0;
    int layout_wrap_h = 0;

    int children_available_w = layout_available_w - params->padding_left - params->padding_right;
    int children_available_h = layout_available_h - params->padding_top - params->padding_bottom;
    int children_wrap_w = 0;
    int children_wrap_h = 0;

    int n_remainings = LinkedListGetLength(layout->children);
    LinkedListEntry *entry = LinkedListHead(layout->children);

    // 先更新非MATH_PARENT的子控件，不然MATH_PARENT会占掉所有的空间
    while (entry)
    {
        void *c_view = LinkedListGetEntryData(entry);
        LayoutParams *c_params = LayoutParamsGetParams(c_view);
        if (params->orientation == TYPE_LAYOUT_PARAMS_ORIENTATION_FRAME ||
            (params->orientation == TYPE_LAYOUT_PARAMS_ORIENTATION_HORIZONTAL && c_params->layout_w != TYPE_LAYOUT_PARAMS_MATH_PARENT) ||
            (params->orientation == TYPE_LAYOUT_PARAMS_ORIENTATION_VERTICAL && c_params->layout_h != TYPE_LAYOUT_PARAMS_MATH_PARENT))
        {
            LayoutUpdateChild(layout, c_view, &children_available_w, &children_available_h, &children_wrap_w, &children_wrap_h);
            n_remainings--;
        }
        entry = LinkedListNext(entry);
    }

    // 再更新MATH_PARENT的子控件，第一个MATH_PARENT会占用掉所有空间，以至于后面的视图将无法显示，暂时不支持weight属性
    if (n_remainings > 0)
    {
        entry = LinkedListHead(layout->children);
        while (entry)
        {
            void *c_view = LinkedListGetEntryData(entry);
            LayoutParams *c_params = LayoutParamsGetParams(c_view);
            if ((params->orientation == TYPE_LAYOUT_PARAMS_ORIENTATION_HORIZONTAL && c_params->layout_w == TYPE_LAYOUT_PARAMS_MATH_PARENT) ||
                (params->orientation == TYPE_LAYOUT_PARAMS_ORIENTATION_VERTICAL && c_params->layout_h == TYPE_LAYOUT_PARAMS_MATH_PARENT))
            {
                LayoutUpdateChild(layout, c_view, &children_available_w, &children_available_h, &children_wrap_w, &children_wrap_h);
            }
            entry = LinkedListNext(entry);
        }
    }

    layout_wrap_w = children_wrap_w + params->padding_left + params->padding_right;
    layout_wrap_h = children_wrap_h + params->padding_top + params->padding_bottom;

    params->wrap_w = layout_wrap_w;
    params->wrap_h = layout_wrap_h;

    if (params->layout_w == TYPE_LAYOUT_PARAMS_MATH_PARENT)
        params->measured_w = layout_available_w;
    else if (params->layout_w == TYPE_LAYOUT_PARAMS_WRAP_CONTENT)
        params->measured_w = layout_wrap_w;
    else
        params->measured_w = params->layout_w;
    if (params->measured_w > layout_available_w)
        params->measured_w = layout_available_w;
    if (params->measured_w < 0)
        params->measured_w = 0;

    if (params->layout_h == TYPE_LAYOUT_PARAMS_MATH_PARENT)
        params->measured_h = layout_available_h;
    else if (params->layout_h == TYPE_LAYOUT_PARAMS_WRAP_CONTENT)
        params->measured_h = layout_wrap_h;
    else
        params->measured_h = params->layout_h;
    if (params->measured_h > layout_available_h)
        params->measured_h = layout_available_h;
    if (params->measured_h < 0)
        params->measured_h = 0;

    return 0;
}

static int LayoutDrawChild(Layout *layout, void *view, int *x, int *y, int min_x, int min_y, int max_x, int max_y)
{
    if (!layout || !view)
        return 0;

    LayoutParams *l_params = LayoutParamsGetParams(layout);
    LayoutParams *c_params = LayoutParamsGetParams(view);

    int occupy_w = c_params->measured_w + c_params->margin_left + c_params->margin_right; // 视图实际所占的空间包括了margin，而measured不包括，所以需要加上
    int occupy_h = c_params->measured_h + c_params->margin_top + c_params->margin_bottom;
    int sx = *x;
    int sy = *y;
    int next_x = *x;
    int next_y = *y;

    if ((l_params->gravity & TYPE_LAYOUT_PARAMS_GRAVITY_BOTTOM) && occupy_h < l_params->measured_h) // 子控件居底
    {
        if (l_params->orientation == TYPE_LAYOUT_PARAMS_ORIENTATION_HORIZONTAL || l_params->orientation == TYPE_LAYOUT_PARAMS_ORIENTATION_FRAME) // 仅水平布局或帧布局
            sy = max_y - occupy_h;
    }

    if ((l_params->gravity & TYPE_LAYOUT_PARAMS_GRAVITY_RIGHT) && occupy_w < l_params->measured_w) // 子控件居右
    {
        if (l_params->orientation == TYPE_LAYOUT_PARAMS_ORIENTATION_VERTICAL || l_params->orientation == TYPE_LAYOUT_PARAMS_ORIENTATION_FRAME) // 仅垂直布局或帧布局
            sx = max_x - occupy_w;
    }

    int dx = sx + occupy_w;
    int dy = sy + occupy_h;

    if (l_params->orientation == TYPE_LAYOUT_PARAMS_ORIENTATION_HORIZONTAL)
    {
        // 水平布局: x 前进，y 不变
        next_x = dx;
        if (dx <= min_x) // 当前控件未在视图显示区域，跳过绘制
            goto NEXT;
        if (sx >= max_x) // 当前控件超出了视图显示区域，返回终止提示
            return -1;
    }
    else if (l_params->orientation == TYPE_LAYOUT_PARAMS_ORIENTATION_VERTICAL)
    {
        // 垂直布局: y 前进，x 不变
        next_y = dy;
        if (dy <= min_y)
            goto NEXT;
        if (sy >= max_y)
            return -1;
    }

    LayoutParamsSetLayoutPosition(view, sx, sy);
    LayoutParamsDraw(view);

NEXT:
    *x = next_x;
    *y = next_y;

    return 0;
}

static int LayoutDraw(void *view)
{
    if (!view)
        return -1;

    Layout *layout = (Layout *)view;
    LayoutParams *params = LayoutParamsGetParams(layout);

    if (params->measured_w <= 0 || params->measured_h <= 0)
        return 0;

    int layout_x = params->layout_x + params->margin_left;
    int layout_y = params->layout_y + params->margin_top;
    int layout_w = params->measured_w;
    int layout_h = params->measured_h;

    int children_sx = layout_x + params->padding_left;
    int children_sy = layout_y + params->padding_top;
    int children_dx = layout_x + layout_w - params->padding_right;
    int children_dy = layout_y + layout_h - params->padding_bottom;
    int children_w = children_dx - children_sx;
    int children_h = children_dy - children_sy;

    if (layout->bg_color)
        GUI_DrawFillRectangle(layout_x, layout_y, layout_w, layout_h, layout->bg_color);

    GUI_SetClipping(children_sx, children_sy, children_w, children_h);

    int child_x = children_sx;
    int child_y = children_sy;

    LinkedListEntry *entry = LinkedListHead(layout->children);
    while (entry)
    {
        LinkedListEntry *next = LinkedListNext(entry);
        void *view = LinkedListGetEntryData(entry);
        if (LayoutDrawChild(layout, view, &child_x, &child_y, children_sx, children_sy, children_dx, children_dy) < 0)
            break;
        entry = next;
    }

    GUI_UnsetClipping();

    return 0;
}

int LayoutAddView(Layout *layout, void *view)
{
    if (!layout || !view)
        return 0;

    if (LinkedListAdd(layout->children, view))
        return 1;

    return 0;
}

int LayoutAddViewAbove(Layout *layout, void *view, void *above)
{
    if (!layout || !view || !above)
        return 0;

    LinkedListEntry *a_entry = LinkedListFindByData(layout->children, above);
    if (a_entry)
    {
        if (LinkedListAddAbove(layout->children, a_entry, view))
            return 1;
    }

    return 0;
}

int LayoutAddViewBelow(Layout *layout, void *view, void *below)
{
    if (!layout || !view || !below)
        return 0;

    LinkedListEntry *b_entry = LinkedListFindByData(layout->children, below);
    if (b_entry)
    {
        if (LinkedListAddBelow(layout->children, b_entry, view))
            return 1;
    }

    return 0;
}

int LayoutRemoveView(Layout *layout, void *view)
{
    if (!layout || !view)
        return 0;

    LinkedListEntry *entry = LinkedListFindByData(layout->children, view);
    if (entry)
    {
        LinkedListRemove(layout->children, entry);
        return 1;
    }

    return 0;
}

int LayoutEmpty(Layout *layout)
{
    if (!layout)
        return -1;

    LinkedListEmpty(layout->children);
    return 0;
}

int LayoutSetBgColor(Layout *layout, uint32_t color)
{
    if (!layout)
        return -1;

    layout->bg_color = color;
    return 0;
}

int LayoutSetData(Layout *layout, void *data)
{
    if (!layout)
        return -1;

    layout->userdata = data;
    return 0;
}

void *LayoutGetData(Layout *layout)
{
    return layout ? layout->userdata : NULL;
}

Layout *NewLayout()
{
    Layout *layout = (Layout *)malloc(sizeof(Layout));
    if (!layout)
        return NULL;
    memset(layout, 0, sizeof(Layout));

    layout->children = NewLinkedList();
    if (!layout->children)
    {
        free(layout);
        return NULL;
    }
    LinkedListSetFreeEntryDataCallback(layout->children, LayoutParamsDestroy);

    LayoutParams *params = LayoutParamsGetParams(layout);
    params->destroy = LayoutDestroy;
    params->update = LayoutUpdate;
    params->draw = LayoutDraw;

    return layout;
}
