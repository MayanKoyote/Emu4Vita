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
    ImageView *iconView;
    TextView *nameView;
    TextView *infoView;
    const GUI_Texture *icon_tex;
    const char *name_text;
    const char *info_text;
    uint32_t icon_tint_color;
    uint32_t name_text_color;
    uint32_t info_text_color;
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
    itemView->iconView = NULL;
    itemView->nameView = NULL;

    if (params->available_w <= 0 || params->available_h <= 0)
    {
        params->measured_w = 0;
        params->measured_h = 0;
        return -1;
    }

    int view_max_h = params->available_h - params->margin_top - params->margin_bottom;
    int view_available_h = view_max_h - params->padding_top - params->padding_bottom;

    if (itemView->icon_tex)
    {
        int icon_layout_h = GUI_GetLineHeight() * 2;
        if (icon_layout_h > view_available_h)
            icon_layout_h = view_available_h;

        itemView->iconView = NewImageView();
        LayoutParamsSetLayoutSize(itemView->iconView, icon_layout_h, icon_layout_h);
        ImageViewSetTexture(itemView->iconView, itemView->icon_tex);
        ImageViewSetScaleType(itemView->iconView, TYPE_IMAGE_SCALE_FIT_CENTER_INSIDE);
        ImageViewSetTintColor(itemView->iconView, itemView->icon_tint_color);
    }

    if (itemView->name_text)
    {
        itemView->nameView = NewTextView();
        LayoutParamsSetLayoutSize(itemView->nameView, TYPE_LAYOUT_PARAMS_MATH_PARENT, TYPE_LAYOUT_PARAMS_WRAP_CONTENT);
        if (itemView->iconView)
            LayoutParamsSetMargin(itemView->nameView, ITEMVIEW_CHILD_MARGIN, 0, 0, 0);
        TextViewSetText(itemView->nameView, itemView->name_text);
        TextViewSetSingleLine(itemView->nameView, 1);
        TextViewSetTextColor(itemView->nameView, itemView->name_text_color);
    }

    if (itemView->info_text)
    {
        // 待添加
    }

    itemView->layout = NewLayout();
    // 从itemView传入layout
    LayoutParamsSetOrientation(itemView->layout, TYPE_LAYOUT_PARAMS_ORIENTATION_HORIZONTAL);
    LayoutParamsSetAvailableSize(itemView->layout, params->available_w, params->available_h);
    LayoutParamsSetLayoutSize(itemView->layout, params->layout_w, params->layout_h);
    LayoutParamsSetMargin(itemView->layout, params->margin_left, params->margin_right, params->margin_top, params->margin_bottom);
    LayoutParamsSetPadding(itemView->layout, params->padding_left, params->padding_right, params->padding_top, params->padding_bottom);
    LayoutSetBgColor(itemView->layout, itemView->bg_color);

    LayoutAddView(itemView->layout, itemView->iconView);
    LayoutAddView(itemView->layout, itemView->nameView);
    LayoutAddView(itemView->layout, itemView->infoView);
    LayoutParamsUpdate(itemView->layout);

    // 从layout回传itemView
    LayoutParamsGetMeasuredSize(itemView->layout, &params->measured_w, &params->measured_h);
    LayoutParamsGetWrapSize(itemView->layout, &params->wrap_w, &params->wrap_h);

    return 0;
}

int ItemViewDraw(void *view)
{
    if (!view)
        return -1;

    ItemView *itemView = (ItemView *)view;
    LayoutParams *params = LayoutParamsGetParams(itemView);

    // 从itemView传入layout
    LayoutParamsSetLayoutPosition(itemView->layout, params->layout_x, params->layout_y);
    return LayoutParamsDraw(itemView->layout);
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

int ItemViewSetIconTexture(ItemView *itemView, const GUI_Texture *texture)
{
    if (!itemView)
        return -1;

    itemView->icon_tex = texture;
    if (itemView->iconView)
        ImageViewSetTexture(itemView->iconView, texture);

    return 0;
}

int ItemViewSetNameText(ItemView *itemView, const char *text)
{
    if (!itemView)
        return -1;

    itemView->name_text = text;
    if (itemView->nameView)
        TextViewSetText(itemView->nameView, text);

    return 0;
}

int ItemViewSetInfoText(ItemView *itemView, const char *text)
{
    if (!itemView)
        return -1;

    itemView->info_text = text;
    if (itemView->infoView)
        TextViewSetText(itemView->infoView, text);

    return 0;
}

int ItemViewSetIconTintColor(ItemView *itemView, uint32_t color)
{
    if (!itemView)
        return -1;

    itemView->icon_tint_color = color;
    if (itemView->iconView)
        ImageViewSetTintColor(itemView->iconView, color);

    return 0;
}

int ItemViewSetNameTextColor(ItemView *itemView, uint32_t color)
{
    if (!itemView)
        return -1;

    itemView->name_text_color = color;
    if (itemView->nameView)
        TextViewSetTextColor(itemView->nameView, color);

    return 0;
}

int ItemViewSetInfoTextColor(ItemView *itemView, uint32_t color)
{
    if (!itemView)
        return -1;

    itemView->info_text_color = color;
    if (itemView->infoView)
        TextViewSetTextColor(itemView->infoView, color);

    return 0;
}

void *ItemViewGetData(ItemView *itemView)
{
    return itemView ? itemView->data : NULL;
}

ItemView *NewItemView()
{
    ItemView *itemView = (ItemView *)malloc(sizeof(ItemView));
    if (!itemView)
        return NULL;
    memset(itemView, 0, sizeof(ItemView));

    LayoutParams *params = LayoutParamsGetParams(itemView);
    params->destroy = ItemViewDestroy;
    params->update = ItemViewUpdate;
    params->draw = ItemViewDraw;

    return itemView;
}
