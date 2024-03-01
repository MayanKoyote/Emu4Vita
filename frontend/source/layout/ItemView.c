#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "gui/gui.h"
#include "Layout.h"

#define ITEMVIEW_CHILD_MARGIN 4

struct ItemView
{
    LayoutParams params;
    uint32_t bg_color;
    Layout *layout;
    ImageView *imageView;
    TextView *nameView;
    TextView *infoView;
    const GUI_Texture *image;
    const char *name;
    const char *info;
    uint32_t image_tint_color;
    uint32_t name_color;
    uint32_t info_color;
    void *data;
};

void ItemViewDestroy(void *view)
{
    ItemView *itemView = (ItemView *)view;
    LayoutParams *params = LayoutParamsGetParams(itemView);

    if (!itemView || params->dont_free)
        return;

    LayoutParamsDestroy(itemView->layout);
    free(itemView);
}

int ItemViewUpdate(void *view)
{
    if (!view)
        return -1;

    ItemView *itemView = (ItemView *)view;
    LayoutParams *params = LayoutParamsGetParams(itemView);

    LayoutParamsDestroy(itemView->layout);
    itemView->layout = NULL;
    itemView->imageView = NULL;
    itemView->nameView = NULL;

    if (params->available_w <= 0 || params->available_h <= 0)
    {
        params->measured_w = 0;
        params->measured_h = 0;
        return -1;
    }

    int view_max_h = params->available_h - params->margin_top - params->margin_bottom;
    int view_available_h = view_max_h - params->padding_top - params->padding_bottom;

    if (itemView->image)
    {
        int image_layout_h = GUI_GetLineHeight() * 2;
        if (image_layout_h > view_available_h)
            image_layout_h = view_available_h;

        itemView->imageView = NewImageView();
        LayoutParamsSetLayoutSize(itemView->imageView, image_layout_h, image_layout_h);
        ImageViewSetTexture(itemView->imageView, itemView->image);
        ImageViewSetScaleType(itemView->imageView, TYPE_IMAGE_SCALE_FIT_CENTER_INSIDE);
        ImageViewSetTintColor(itemView->imageView, itemView->image_tint_color);
    }

    if (itemView->name)
    {
        itemView->nameView = NewTextView();
        LayoutParamsSetLayoutSize(itemView->nameView, TYPE_LAYOUT_MATH_PARENT, TYPE_LAYOUT_WRAP_CONTENT);
        if (itemView->imageView)
            LayoutParamsSetMargin(itemView->nameView, ITEMVIEW_CHILD_MARGIN, 0, 0, 0);
        TextViewSetText(itemView->nameView, itemView->name);
        TextViewSetSingleLine(itemView->nameView, 1);
        TextViewSetTextColor(itemView->nameView, itemView->name_color);
    }

    itemView->layout = NewLayout();
    // 从itemView传入layout
    LayoutParamsSetOrientation(itemView->layout, TYPE_LAYOUT_ORIENTATION_HORIZONTAL);
    LayoutParamsSetAvailableSize(itemView->layout, params->available_w, params->available_h);
    LayoutParamsSetLayoutSize(itemView->layout, params->layout_w, params->layout_h);
    LayoutParamsSetMargin(itemView->layout, params->margin_left, params->margin_right, params->margin_top, params->margin_bottom);
    LayoutParamsSetPadding(itemView->layout, params->padding_left, params->padding_right, params->padding_top, params->padding_bottom);
    LayoutSetBgColor(itemView->layout, itemView->bg_color);

    LayoutAddView(itemView->layout, itemView->imageView);
    LayoutAddView(itemView->layout, itemView->nameView);
    LayoutParamsUpdate(itemView->layout);

    // 从layout回传itemView
    LayoutParamsGetMeasuredSize(itemView->layout, &params->measured_w, &params->measured_h);
    LayoutParamsGetWrapSize(itemView->layout, &params->wrap_w, &params->wrap_h);

    return 0;
}

void ItemViewDraw(void *view)
{
    if (!view)
        return;

    ItemView *itemView = (ItemView *)view;
    LayoutParams *params = LayoutParamsGetParams(itemView);

    LayoutParamsSetLayoutPosition(itemView->layout, params->layout_x, params->layout_y);
    LayoutParamsDraw(itemView->layout);
}

int ItemViewSetBgColor(ItemView *itemView, uint32_t color)
{
    if (!itemView)
        return -1;

    itemView->bg_color = color;
    if (itemView->layout)
        LayoutSetBgColor(itemView->layout, color);

    return 0;
}

int ItemViewSetData(ItemView *itemView, void *data)
{
    if (!itemView)
        return -1;

    itemView->data = data;
    return 0;
}

int ItemViewSetImage(ItemView *itemView, const GUI_Texture *image)
{
    if (!itemView)
        return -1;

    itemView->image = image;
    if (itemView->imageView)
        ImageViewSetTexture(itemView->imageView, image);

    return 0;
}

int ItemViewSetName(ItemView *itemView, const char *name)
{
    if (!itemView)
        return -1;

    itemView->name = name;
    if (itemView->nameView)
        TextViewSetText(itemView->nameView, name);

    return 0;
}

int ItemViewSetInfo(ItemView *itemView, const char *info)
{
    if (!itemView)
        return -1;

    itemView->info = info;
    if (itemView->infoView)
        TextViewSetText(itemView->infoView, info);

    return 0;
}

int ItemViewSetImageTintColor(ItemView *itemView, uint32_t color)
{
    if (!itemView)
        return -1;

    itemView->image_tint_color = color;
    if (itemView->imageView)
        ImageViewSetTintColor(itemView->imageView, color);

    return 0;
}

int ItemViewSetNameColor(ItemView *itemView, uint32_t color)
{
    if (!itemView)
        return -1;

    itemView->name_color = color;
    if (itemView->nameView)
        TextViewSetTextColor(itemView->nameView, color);

    return 0;
}

int ItemViewSetInfoColor(ItemView *itemView, uint32_t color)
{
    if (!itemView)
        return -1;

    itemView->info_color = color;
    if (itemView->infoView)
        TextViewSetTextColor(itemView->infoView, color);

    return 0;
}

void *ItemViewGetData(ItemView *itemView)
{
    return itemView ? itemView->data : NULL;
}

int ItemViewInit(ItemView *itemView)
{
    if (!itemView)
        return -1;

    memset(itemView, 0, sizeof(ItemView));

    LayoutParams *params = LayoutParamsGetParams(itemView);
    params->destroy = ItemViewDestroy;
    params->update = ItemViewUpdate;
    params->draw = ItemViewDraw;

    return 0;
}

ItemView *NewItemView()
{
    ItemView *itemView = (ItemView *)malloc(sizeof(ItemView));
    if (!itemView)
        return NULL;

    ItemViewInit(itemView);

    return itemView;
}
