#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "gui/gui.h"
#include "Layout.h"

struct RectView
{
    LayoutParams params;
    uint32_t bg_color;
    uint32_t rect_color;
    void *userdata;
};

static void RectViewDestroy(void *view)
{
    RectView *rectView = (RectView *)view;
    LayoutParams *params = LayoutParamsGetParams(rectView);

    if (!rectView || params->dont_free)
        return;

    free(rectView);
}

static int RectViewUpdate(void *view)
{
    if (!view)
        return -1;

    RectView *rectView = (RectView *)view;
    LayoutParams *params = LayoutParamsGetParams(rectView);

    if (params->available_w <= 0 || params->available_h <= 0)
    {
        params->measured_w = 0;
        params->measured_h = 0;
        return -1;
    }

    int layout_available_w = params->available_w - params->margin_left - params->margin_right; // 最大宽度
    int layout_available_h = params->available_h - params->margin_top - params->margin_bottom; // 最大高度
    int layout_wrap_w = params->padding_left + params->padding_right;                    // 包裹宽度
    int layout_wrap_h = params->padding_top + params->padding_bottom;                    // 包裹高度

    params->wrap_w = layout_wrap_w;
    params->wrap_h = layout_wrap_h;

    // 测量宽度（绘制时确定的宽度）
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

    // 测量高度（绘制时确定的高度）
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

static int RectViewDraw(void *view)
{
    if (!view)
        return -1;

    RectView *rectView = (RectView *)view;
    LayoutParams *params = LayoutParamsGetParams(rectView);

    if (params->measured_w <= 0 || params->measured_h <= 0)
        return 0;

    int layout_x = params->layout_x + params->margin_left;
    int layout_y = params->layout_y + params->margin_top;
    int layout_w = params->measured_w;
    int layout_h = params->measured_h;

    if (rectView->bg_color)
        GUI_DrawFillRectangle(layout_x, layout_y, layout_w, layout_h, rectView->bg_color);

    if (rectView->rect_color)
    {
        int rect_x = layout_x + params->padding_left;
        int rect_y = layout_y + params->padding_top;
        int rect_w = layout_w - params->padding_left - params->padding_right;
        int rect_h = layout_h - params->padding_top - params->padding_bottom;

        GUI_DrawFillRectangle(rect_x, rect_y, rect_w, rect_h, rectView->rect_color);
    }

    return 0;
}

int RectViewSetBgColor(RectView *rectView, uint32_t color)
{
    if (!rectView)
        return -1;

    rectView->bg_color = color;
    return 0;
}

int RectViewSetData(RectView *rectView, void *data)
{
    if (!rectView)
        return -1;

    rectView->userdata = data;
    return 0;
}

int RectViewSetRectColor(RectView *rectView, uint32_t color)
{
    if (!rectView)
        return -1;

    rectView->rect_color = color;

    return 0;
}

void *RectViewGetData(RectView *rectView)
{
    return rectView ? rectView->userdata : NULL;
}

RectView *NewRectView()
{
    RectView *rectView = (RectView *)malloc(sizeof(RectView));
    if (!rectView)
        return NULL;
    memset(rectView, 0, sizeof(RectView));

    LayoutParams *params = LayoutParamsGetParams(rectView);
    params->destroy = RectViewDestroy;
    params->update = RectViewUpdate;
    params->draw = RectViewDraw;

    return rectView;
}
