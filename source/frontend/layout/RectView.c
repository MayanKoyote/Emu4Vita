#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "gui/gui.h"
#include "Layout.h"
#include "RectView.h"

void RectViewDestroy(void *view)
{
    RectView *rectView = (RectView *)view;

    if (!rectView || rectView->params.dont_free)
        return;

    free(rectView);
}

int RectViewUpdate(void *view, int max_w, int max_h)
{
    if (!view)
        return -1;

    RectView *rectView = (RectView *)view;
    LayoutParam *params = &rectView->params;

    max_w -= (params->margin_left + params->margin_right);
    max_h -= (params->margin_top + params->margin_bottom);

    params->render_w = params->layout_w;
    params->render_h = params->layout_h;

    if (params->layout_w == TYPE_LAYOUT_MATH_PARENT)
        params->render_w = max_w;
    else if (params->layout_w == TYPE_LAYOUT_WRAP_CONTENT)
        params->render_w = 0;
    if (params->render_w > max_w)
        params->render_w = max_w;
    if (params->render_w < 0)
        params->render_w = 0;

    if (params->layout_h == TYPE_LAYOUT_MATH_PARENT)
        params->render_h = max_h;
    else if (params->layout_h == TYPE_LAYOUT_WRAP_CONTENT)
        params->render_h = 0;
    if (params->render_h > max_h)
        params->render_h = max_h;
    if (params->render_h < 0)
        params->render_h = 0;

    return 0;
}

void RectViewDraw(void *view, int x, int y)
{
    if (!view)
        return;

    RectView *rectView = (RectView *)view;
    LayoutParam *params = &rectView->params;

    if (params->render_w <= 0 || params->render_h <= 0)
        return;

    int view_x = x + params->margin_left;
    int view_y = y + params->margin_top;

    if (rectView->bg_color)
        GUI_DrawFillRectangle(view_x, view_y, params->render_w, params->render_h, rectView->bg_color);

    if (rectView->rect_color)
    {
        int rect_x = view_x + params->padding_left;
        int rect_y = view_y + params->padding_top;
        int rect_w = params->render_w - params->padding_left - params->padding_right;
        int rect_h = params->render_h - params->padding_top - params->padding_bottom;

        GUI_DrawFillRectangle(rect_x, rect_y, rect_w, rect_h, rectView->rect_color);
    }
}

int RectViewSetBgColor(RectView *rectView, uint32_t color)
{
    if (!rectView)
        return -1;

    rectView->bg_color = color;

    return 0;
}

int RectViewSetRectColor(RectView *rectView, uint32_t color)
{
    if (!rectView)
        return -1;

    rectView->rect_color = color;

    return 0;
}

int RectViewInit(RectView *rectView)
{
    if (!rectView)
        return -1;

    memset(rectView, 0, sizeof(RectView));

    LayoutParam *params = &rectView->params;
    params->destroy = RectViewDestroy;
    params->update = RectViewUpdate;
    params->draw = RectViewDraw;

    return 0;
}

RectView *NewRectView()
{
    RectView *rectView = (RectView *)malloc(sizeof(RectView));
    if (!rectView)
        return NULL;

    RectViewInit(rectView);

    return rectView;
}
