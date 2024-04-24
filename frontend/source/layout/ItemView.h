#ifndef __M_ITEM_VIEW_H__
#define __M_ITEM_VIEW_H__

#include "gui/gui.h"
#include "Layout.h"

typedef struct ItemView ItemView;

int ItemViewSetBgColor(ItemView *itemView, uint32_t color);
int ItemViewSetData(ItemView *itemView, void *data);
int ItemViewSetIconTexture(ItemView *itemView, const GUI_Texture *texture);
int ItemViewSetNameText(ItemView *itemView, const char *text);
int ItemViewSetInfoText(ItemView *itemView, const char *text);
int ItemViewSetIconTintColor(ItemView *itemView, uint32_t color);
int ItemViewSetNameTextColor(ItemView *itemView, uint32_t color);
int ItemViewSetInfoTextColor(ItemView *itemView, uint32_t color);
int ItemViewSetTextScollEnabled(ItemView *itemView, int enabled);

void *ItemViewGetData(ItemView *itemView);

ItemView *NewItemView();

#endif