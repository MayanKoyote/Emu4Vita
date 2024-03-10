#ifndef __M_LAYOUT_H__
#define __M_LAYOUT_H__

#include "list/linked_list.h"
#include "gui/gui.h"
#include "LayoutParams.h"

typedef struct Layout Layout;

int LayoutAddView(Layout *layout, void *view);
int LayoutAddViewAbove(Layout *layout, void *view, void *above);
int LayoutAddViewBelow(Layout *layout, void *view, void *below);
int LayoutRemoveView(Layout *layout, void *view);
int LayoutEmpty(Layout *layout);

int LayoutSetBgColor(Layout *layout, uint32_t color);
int LayoutSetData(Layout *layout, void *data);

void *LayoutGetData(Layout *layout);

Layout *NewLayout();

#include "RectView.h"
#include "TextView.h"
#include "ImageView.h"
#include "ListView.h"
#include "ItemView.h"

#endif
