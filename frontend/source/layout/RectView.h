#ifndef __M_RECT_VIEW_H__
#define __M_RECT_VIEW_H__

#include "Layout.h"

typedef struct RectView RectView;

int RectViewSetBgColor(RectView *rectView, uint32_t color);
int RectViewSetData(RectView *rectView, void *data);
int RectViewSetRectColor(RectView *rectView, uint32_t color);

void *RectViewGetData(RectView *rectView);

RectView *NewRectView();

#endif
