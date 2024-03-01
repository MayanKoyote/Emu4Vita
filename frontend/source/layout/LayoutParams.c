#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "LayoutParams.h"

LayoutParams *NewLayoutParams()
{
    return (LayoutParams *)calloc(1, sizeof(LayoutParams));
}

LayoutParams *LayoutParamsGetParams(void *view)
{
    return (LayoutParams *)view;
}

int LayoutParamsSetAutoFree(void *view, int auto_free)
{
    if (!view)
        return -1;

    LayoutParams *params = (LayoutParams *)view;
    params->dont_free = !auto_free;

    return 0;
}

int LayoutParamsSetOrientation(void *view, int orientation)
{
    if (!view)
        return -1;

    LayoutParams *params = (LayoutParams *)view;
    params->orientation = orientation;

    return 0;
}

int LayoutParamsSetScrollEnabled(void *view, int enabled)
{
    if (!view)
        return -1;

    LayoutParams *params = (LayoutParams *)view;
    params->scroll_enabled = enabled;

    return 0;
}

int LayoutParamsSetMargin(void *view, int left, int right, int top, int bottom)
{
    if (!view)
        return -1;

    LayoutParams *params = (LayoutParams *)view;
    params->margin_left = left;
    params->margin_right = right;
    params->margin_top = top;
    params->margin_bottom = bottom;

    return 0;
}

int LayoutParamsSetPadding(void *view, int left, int right, int top, int bottom)
{
    if (!view)
        return -1;

    LayoutParams *params = (LayoutParams *)view;
    params->padding_left = left;
    params->padding_right = right;
    params->padding_top = top;
    params->padding_bottom = bottom;

    return 0;
}

int LayoutParamsSetLayoutPosition(void *view, int layout_x, int layout_y)
{
    if (!view)
        return -1;

    LayoutParams *params = (LayoutParams *)view;
    params->layout_x = layout_x;
    params->layout_y = layout_y;

    return 0;
}

int LayoutParamsSetAvailableSize(void *view, int available_w, int available_h)
{
    if (!view)
        return -1;

    LayoutParams *params = (LayoutParams *)view;
    params->available_w = available_w;
    params->available_h = available_h;

    return 0;
}

int LayoutParamsSetLayoutSize(void *view, int layout_w, int layout_h)
{
    if (!view)
        return -1;

    LayoutParams *params = (LayoutParams *)view;
    params->layout_w = layout_w;
    params->layout_h = layout_h;

    return 0;
}

int LayoutParamsGetLayoutPosition(void *view, int *layout_x, int *layout_y)
{
    if (!view)
        return -1;

    LayoutParams *params = (LayoutParams *)view;
    if (layout_x)
        *layout_x = params->layout_x;
    if (layout_y)
        *layout_y = params->layout_y;

    return 0;
}

int LayoutParamsGetAvailableSize(void *view, int *available_w, int *available_h)
{
    if (!view)
        return -1;

    LayoutParams *params = (LayoutParams *)view;
    if (available_w)
        *available_w = params->available_w;
    if (available_h)
        *available_h = params->available_h;

    return 0;
}

int LayoutParamsGetLayoutSize(void *view, int *layout_w, int *layout_h)
{
    if (!view)
        return -1;

    LayoutParams *params = (LayoutParams *)view;
    if (layout_w)
        *layout_w = params->layout_w;
    if (layout_h)
        *layout_h = params->layout_h;

    return 0;
}

int LayoutParamsGetMeasuredSize(void *view, int *measured_w, int *measured_h)
{
    if (!view)
        return -1;

    LayoutParams *params = (LayoutParams *)view;
    if (measured_w)
        *measured_w = params->measured_w;
    if (measured_h)
        *measured_h = params->measured_h;

    return 0;
}

int LayoutParamsGetWrapSize(void *view, int *wrap_w, int *wrap_h)
{
    if (!view)
        return -1;

    LayoutParams *params = (LayoutParams *)view;
    if (wrap_w)
        *wrap_w = params->wrap_w;
    if (wrap_h)
        *wrap_h = params->wrap_h;

    return 0;
}

void LayoutParamsDestroy(void *view)
{
    if (!view)
        return;

    LayoutParams *params = (LayoutParams *)view;
    if (params->destroy)
        params->destroy(view);
}

int LayoutParamsUpdate(void *view)
{
    if (!view)
        return -1;

    LayoutParams *params = (LayoutParams *)view;
    if (params->update)
        return params->update(view);

    return -1;
}

void LayoutParamsDraw(void *view)
{
    if (!view)
        return;

    LayoutParams *params = (LayoutParams *)view;
    if (params->draw)
        params->draw(view);
}
