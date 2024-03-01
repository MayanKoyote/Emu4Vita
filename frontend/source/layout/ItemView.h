#ifndef __M_ITEM_VIEW_H__
#define __M_ITEM_VIEW_H__

#include "gui/gui.h"
#include "Layout.h"

typedef struct ItemView ItemView;

int ItemViewSetBgColor(ItemView *itemView, uint32_t color);
int ItemViewSetData(ItemView *itemView, void *data);
int ItemViewSetImage(ItemView *itemView, const GUI_Texture *image);
int ItemViewSetName(ItemView *itemView, const char *name);
int ItemViewSetInfo(ItemView *itemView, const char *info);
int ItemViewSetImageTintColor(ItemView *itemView, uint32_t color);
int ItemViewSetNameColor(ItemView *itemView, uint32_t color);
int ItemViewSetInfoColor(ItemView *itemView, uint32_t color);

void *ItemViewGetData(ItemView *itemView);

int ItemViewInit(ItemView *itemView);
ItemView *NewItemView();

#endif