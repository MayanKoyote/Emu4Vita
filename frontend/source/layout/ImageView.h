#ifndef __M_IMAGE_VIEW_H__
#define __M_IMAGE_VIEW_H__

#include "gui/gui.h"
#include "Layout.h"

enum TypeImageViewScale
{
    TYPE_IMAGE_SCALE_CENTER,
    TYPE_IMAGE_SCALE_CENTER_CROP,
    TYPE_IMAGE_SCALE_CENTER_INSIDE,
    TYPE_IMAGE_SCALE_FIT_XY,
    TYPE_IMAGE_SCALE_FIT_START,
    TYPE_IMAGE_SCALE_FIT_END,
    TYPE_IMAGE_SCALE_FIT_CENTER,
    TYPE_IMAGE_SCALE_FIT_CENTER_CROP,
    TYPE_IMAGE_SCALE_FIT_CENTER_INSIDE,
    TYPE_IMAGE_SCALE_MATRIX,
};

typedef struct ImageView ImageView;

int ImageViewSetBgColor(ImageView *imageView, uint32_t color);
int ImageViewSetData(ImageView *imageView, void *data);
int ImageViewSetScaleType(ImageView *imageView, int type);
int ImageViewSetTintColor(ImageView *imageView, uint32_t color);
int ImageViewSetTexture(ImageView *imageView, const GUI_Texture *texture);

void *ImageViewGetData(ImageView *imageView);
const GUI_Texture *ImageViewGetTexture(ImageView *imageView);

ImageView *NewImageView();

#endif
