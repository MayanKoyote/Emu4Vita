#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "gui/gui.h"
#include "Layout.h"

struct ImageView
{
    LayoutParams params;
    const GUI_Texture *tex;
    int scale_type;
    uint32_t bg_color;
    uint32_t tint_color;
    int tex_ori_w;
    int tex_ori_h;
    int tex_dst_x;
    int tex_dst_y;
    int tex_dst_w;
    int tex_dst_h;
    int tex_src_x;
    int tex_src_y;
    int tex_src_w;
    int tex_src_h;
};

void ImageViewDestroy(void *view)
{
    ImageView *imageView = (ImageView *)view;
    LayoutParams *params = LayoutParamsGetParams(imageView);

    if (!imageView || params->dont_free)
        return;

    free(imageView);
}

int ImageViewUpdate(void *view)
{
    if (!view)
        return -1;

    ImageView *imageView = (ImageView *)view;
    LayoutParams *params = LayoutParamsGetParams(imageView);

    if (params->available_w <= 0 || params->available_h <= 0)
    {
        params->measured_w = 0;
        params->measured_h = 0;
        return -1;
    }

    int view_max_w = params->available_w - params->margin_left - params->margin_right;
    int view_max_h = params->available_h - params->margin_top - params->margin_bottom;
    int view_wrap_w = imageView->tex_ori_w + params->padding_left + params->padding_right;
    int view_wrap_h = imageView->tex_ori_h + params->padding_top + params->padding_bottom;

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

    if (imageView->tex)
    {
        const int tex_max_w = params->measured_w - params->padding_left - params->padding_right;
        const int tex_max_h = params->measured_h - params->padding_top - params->padding_bottom;
        const int tex_ori_w = imageView->tex_ori_w;
        const int tex_ori_h = imageView->tex_ori_h;
        const float aspect_ratio = (float)tex_ori_w / (float)tex_ori_h;

        switch (imageView->scale_type)
        {
        case TYPE_IMAGE_SCALE_CENTER: // 原图，居中，（宽高度小于视图：原图）
                                      //            （宽高度大于视图：裁剪）
            imageView->tex_dst_w = tex_ori_w;
            imageView->tex_dst_h = tex_ori_h;
            imageView->tex_dst_x = (tex_max_w - imageView->tex_dst_w) / 2;
            imageView->tex_dst_y = (tex_max_h - imageView->tex_dst_h) / 2;

            break;

        case TYPE_IMAGE_SCALE_CENTER_CROP: // 等比缩放，居中，（宽高度小于视图：等比放大）
                                           //                （宽高度大于视图：裁剪）
            imageView->tex_dst_w = tex_ori_w;
            imageView->tex_dst_h = tex_ori_h;
            if (imageView->tex_dst_w < tex_max_w)
            {
                imageView->tex_dst_w = tex_max_w;
                imageView->tex_dst_h = imageView->tex_dst_w / aspect_ratio;
            }
            if (imageView->tex_dst_h < tex_max_h)
            {
                imageView->tex_dst_h = tex_max_h;
                imageView->tex_dst_w = imageView->tex_dst_h * aspect_ratio;
            }
            imageView->tex_dst_x = (tex_max_w - imageView->tex_dst_w) / 2;
            imageView->tex_dst_y = (tex_max_h - imageView->tex_dst_h) / 2;

            break;

        case TYPE_IMAGE_SCALE_CENTER_INSIDE: // 等比缩放，居中，（宽高度小于视图： 原图）
                                             //                （宽高度大于视图： 等比缩小）
            imageView->tex_dst_w = tex_ori_w;
            imageView->tex_dst_h = tex_ori_h;
            if (imageView->tex_dst_w > tex_max_w)
            {
                imageView->tex_dst_w = tex_max_w;
                imageView->tex_dst_h = imageView->tex_dst_w / aspect_ratio;
            }
            if (imageView->tex_dst_h > tex_max_h)
            {
                imageView->tex_dst_h = tex_max_h;
                imageView->tex_dst_w = imageView->tex_dst_h * aspect_ratio;
            }
            imageView->tex_dst_x = (tex_max_w - imageView->tex_dst_w) / 2;
            imageView->tex_dst_y = (tex_max_h - imageView->tex_dst_h) / 2;

            break;

        case TYPE_IMAGE_SCALE_FIT_XY: // 拉伸缩放，左上对齐 （宽==视图，高==视图）
            imageView->tex_dst_w = tex_max_w;
            imageView->tex_dst_h = tex_max_h;
            imageView->tex_dst_x = 0;
            imageView->tex_dst_y = 0;

            break;

        case TYPE_IMAGE_SCALE_FIT_START: // 等比缩放，左上对齐，（宽==视图，高等比缩放）
            imageView->tex_dst_w = tex_max_w;
            imageView->tex_dst_h = imageView->tex_dst_w / aspect_ratio;
            imageView->tex_dst_x = 0;
            imageView->tex_dst_y = 0;

            break;

        case TYPE_IMAGE_SCALE_FIT_END: // 等比缩放，左下对齐，（宽==视图，高等比缩放）
            imageView->tex_dst_w = tex_max_w;
            imageView->tex_dst_h = imageView->tex_dst_w / aspect_ratio;
            imageView->tex_dst_x = 0;
            imageView->tex_dst_y = tex_max_h - imageView->tex_dst_h;

            break;

        case TYPE_IMAGE_SCALE_FIT_CENTER: // 等比缩放，垂直居中，（宽==视图，高等比缩放）
            imageView->tex_dst_w = tex_max_w;
            imageView->tex_dst_h = imageView->tex_dst_w / aspect_ratio;
            imageView->tex_dst_x = 0;
            imageView->tex_dst_y = (tex_max_h - imageView->tex_dst_h) / 2;

            break;

        case TYPE_IMAGE_SCALE_FIT_CENTER_CROP: // 等比缩放，居中，（(宽==视图，高>=视图) || (高==视图，宽>=视图））
            imageView->tex_dst_w = tex_max_w;
            imageView->tex_dst_h = imageView->tex_dst_w / aspect_ratio;
            if (imageView->tex_dst_h < tex_max_h)
            {
                imageView->tex_dst_h = tex_max_h;
                imageView->tex_dst_w = imageView->tex_dst_h * aspect_ratio;
            }
            imageView->tex_dst_x = (tex_max_w - imageView->tex_dst_w) / 2;
            imageView->tex_dst_y = (tex_max_h - imageView->tex_dst_h) / 2;

            break;

        case TYPE_IMAGE_SCALE_FIT_CENTER_INSIDE: // 等比缩放，居中，（(宽==视图，高<=视图) || (高==视图，宽<=视图））
            imageView->tex_dst_w = tex_max_w;
            imageView->tex_dst_h = imageView->tex_dst_w / aspect_ratio;
            if (imageView->tex_dst_h > tex_max_h)
            {
                imageView->tex_dst_h = tex_max_h;
                imageView->tex_dst_w = imageView->tex_dst_h * aspect_ratio;
            }
            imageView->tex_dst_x = (tex_max_w - imageView->tex_dst_w) / 2;
            imageView->tex_dst_y = (tex_max_h - imageView->tex_dst_h) / 2;

            break;

        case TYPE_IMAGE_SCALE_MATRIX: // 图像原始大小，左上对齐
        default:
            imageView->tex_dst_w = tex_ori_w;
            imageView->tex_dst_h = tex_ori_h;
            imageView->tex_dst_x = 0;
            imageView->tex_dst_y = 0;

            break;
        }
    }

    return 0;
}

void ImageViewDraw(void *view)
{
    if (!view)
        return;

    ImageView *imageView = (ImageView *)view;
    LayoutParams *params = LayoutParamsGetParams(imageView);

    if (params->measured_w <= 0 || params->measured_h <= 0)
        return;

    int view_x = params->layout_x + params->margin_left;
    int view_y = params->layout_y + params->margin_top;
    int view_max_w = params->measured_w;
    int view_max_h = params->measured_h;

    if (imageView->bg_color)
        GUI_DrawFillRectangle(view_x, view_y, view_max_w, view_max_h, imageView->bg_color);

    if (imageView->tex)
    {
        int tex_x = view_x + params->padding_left;
        int tex_y = view_y + params->padding_top;
        int tex_max_w = view_max_w - params->padding_left - params->padding_right;
        int tex_max_h = view_max_h - params->padding_top - params->padding_bottom;

        GUI_SetClipping(tex_x, tex_y, tex_max_w, tex_max_h);

        int tex_dst_x = tex_x + imageView->tex_dst_x;
        int tex_dst_y = tex_y + imageView->tex_dst_y;
        float tex_scale_x = (float)imageView->tex_dst_w / (float)imageView->tex_ori_w;
        float tex_scale_y = (float)imageView->tex_dst_h / (float)imageView->tex_ori_h;

        if (imageView->tint_color)
            GUI_DrawTextureTintPartScale(imageView->tex, tex_dst_x, tex_dst_y, 0, 0,
                                         imageView->tex_ori_w, imageView->tex_ori_h, tex_scale_x, tex_scale_y, imageView->tint_color);
        else
            GUI_DrawTexturePartScale(imageView->tex, tex_dst_x, tex_dst_y, 0, 0,
                                     imageView->tex_ori_w, imageView->tex_ori_h, tex_scale_x, tex_scale_y);

        GUI_UnsetClipping();
    }
}

int ImageViewSetBgColor(ImageView *imageView, uint32_t color)
{
    if (!imageView)
        return -1;

    imageView->bg_color = color;
    return 0;
}

int ImageViewSetScaleType(ImageView *imageView, int type)
{
    if (!imageView)
        return -1;

    imageView->scale_type = type;
    return 0;
}

int ImageViewSetTintColor(ImageView *imageView, uint32_t color)
{
    if (!imageView)
        return -1;

    imageView->tint_color = color;
    return 0;
}

int ImageViewSetTexture(ImageView *imageView, const GUI_Texture *texture)
{
    if (!imageView)
        return -1;

    imageView->tex = texture;
    imageView->tex_ori_w = 0;
    imageView->tex_ori_h = 0;

    if (imageView->tex)
    {
        imageView->tex_ori_w = GUI_GetTextureWidth(imageView->tex);
        imageView->tex_ori_h = GUI_GetTextureHeight(imageView->tex);
    }

    return 0;
}

const GUI_Texture *ImageViewGetTexture(ImageView *imageView)
{
    return imageView ? imageView->tex : NULL;
}

int ImageViewInit(ImageView *imageView)
{
    if (!imageView)
        return -1;

    memset(imageView, 0, sizeof(ImageView));

    LayoutParams *params = LayoutParamsGetParams(imageView);
    params->destroy = ImageViewDestroy;
    params->update = ImageViewUpdate;
    params->draw = ImageViewDraw;

    return 0;
}

ImageView *NewImageView()
{
    ImageView *imageView = (ImageView *)malloc(sizeof(ImageView));
    if (!imageView)
        return NULL;

    ImageViewInit(imageView);

    return imageView;
}
