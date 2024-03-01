#ifndef __M_RECT_VIEW_H__
#define __M_RECT_VIEW_H__

#include "Layout.h"

typedef struct RectView RectView;

int RectViewSetBgColor(RectView *rectView, uint32_t color);
int RectViewSetRectColor(RectView *rectView, uint32_t color);

int RectViewInit(RectView *rectView);
RectView *NewRectView();

#endif
