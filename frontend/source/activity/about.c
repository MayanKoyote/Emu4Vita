#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "gui/gui.h"
#include "utils.h"
#include "lang.h"

#include "about_text.c"

static int startActivityCallback(GUI_Activity *activity);
static void drawActivityCallback(GUI_Activity *activity);
static void ctrlActivityCallback(GUI_Activity *activity);

static GUI_ButtonInstruction button_instructions[] = {
    {LANG_BUTTON_CANCEL, LANG_BACK, 1},
    {LANG_NULL, LANG_NULL, 1},
};

GUI_Activity about_activity = {
    LANG_ABOUT_TITLE,      // Title
    button_instructions,   // Button instructions
    NULL,                  // Wallpaper
    startActivityCallback, // Start callback
    NULL,                  // Exit callback
    drawActivityCallback,  // Draw callback
    ctrlActivityCallback,  // Ctrl callback
    0,                     // Disable draw statusbar
    NULL,                  // Parent activity
    NULL,                  // User data
};

#define LISTVIEW_PADDING_L 6
#define LISTVIEW_PADDING_T 6
#define ITEMVIEW_PADDING_L 6
#define ITEMVIEW_PADDING_T 6

#define TEXT_COLOR COLOR_ORANGE

static Layout *about_layout = NULL;
static ListView *about_listView = NULL;

static int getListLength(void *list)
{
    return list ? N_ABOUT_TEXTS : 0;
}

static void *getHeadListEntry(void *list)
{
    return list ? ((void **)list)[0] : NULL;
}

static void *getNextListEntry(void *list, void *cur_entry, int cur_idx)
{
    int next_idx = cur_idx + 1;
    if (list && next_idx >= 0 && next_idx <= N_ABOUT_TEXTS - 1)
        return ((void **)list)[next_idx];
    return NULL;
}

static void *newItemView(void *data)
{
    ItemView *itemView = NewItemView();
    if (!itemView)
        return NULL;

    LayoutParamsSetPadding(itemView, ITEMVIEW_PADDING_L, ITEMVIEW_PADDING_L, ITEMVIEW_PADDING_T, ITEMVIEW_PADDING_T);
    LayoutParamsSetLayoutSize(itemView, TYPE_LAYOUT_MATH_PARENT, TYPE_LAYOUT_WRAP_CONTENT);
    ItemViewSetNameColor(itemView, TEXT_COLOR);
    if (data)
    {
        const char *text = (const char *)data;
        ItemViewSetName(itemView, text);
    }

    return itemView;
}

static int setItemViewData(void *itemView, void *data)
{
    return ItemViewSetData((ItemView *)itemView, data);
}

static void refreshLayout()
{
    int layout_x = 0, layout_y = 0;
    int available_w = 0, available_h = 0;
    GUI_GetActivityLayoutPosition(&about_activity, &layout_x, &layout_y);
    GUI_GetActivityAvailableSize(&about_activity, &available_w, &available_h);

    if (!about_layout)
    {
        about_layout = NewLayout();
        if (!about_layout)
            return;
        LayoutParamsSetAutoFree(about_layout, 0);
    }
    LayoutParamsSetOrientation(about_layout, TYPE_LAYOUT_ORIENTATION_VERTICAL);
    LayoutParamsSetLayoutPosition(about_layout, layout_x, layout_y);
    LayoutParamsSetAvailableSize(about_layout, available_w, available_h);
    LayoutParamsSetLayoutSize(about_layout, TYPE_LAYOUT_MATH_PARENT, TYPE_LAYOUT_MATH_PARENT);
    LayoutParamsSetPadding(about_layout, GUI_DEF_MAIN_LAYOUT_PADDING, GUI_DEF_MAIN_LAYOUT_PADDING, GUI_DEF_MAIN_LAYOUT_PADDING, GUI_DEF_MAIN_LAYOUT_PADDING);
    LayoutEmpty(about_layout);

    if (!about_listView)
    {
        about_listView = NewListView();
        if (!about_listView)
            return;
    }
    LayoutParamsSetOrientation(about_listView, TYPE_LAYOUT_ORIENTATION_VERTICAL);
    LayoutParamsSetLayoutSize(about_listView, TYPE_LAYOUT_MATH_PARENT, TYPE_LAYOUT_MATH_PARENT);
    LayoutParamsSetPadding(about_listView, LISTVIEW_PADDING_L, LISTVIEW_PADDING_L, LISTVIEW_PADDING_T, LISTVIEW_PADDING_T);
    ListViewSetBgColor(about_listView, GUI_DEF_COLOR_BG);
    ListViewCallbacks callbacks;
    memset(&callbacks, 0, sizeof(ListViewCallbacks));
    callbacks.newItemView = newItemView;
    callbacks.setItemViewData = setItemViewData;
    callbacks.getListLength = getListLength;
    callbacks.getHeadListEntry = getHeadListEntry;
    callbacks.getNextListEntry = getNextListEntry;
    ListViewSetList(about_listView, about_texts, &callbacks);
    LayoutAddView(about_layout, about_listView);

    LayoutParamsUpdate(about_layout);
}

static int startActivityCallback(GUI_Activity *activity)
{
    about_activity.wallpaper = GUI_GetDefaultWallpaper();
    refreshLayout();
    return 0;
}

static void drawActivityCallback(GUI_Activity *activity)
{
    LayoutParamsDraw(about_layout);
}

static void ctrlActivityCallback(GUI_Activity *activity)
{
    if (hold_pad[PAD_UP] || hold2_pad[PAD_LEFT_ANALOG_UP])
    {
        ListViewMoveTopPos(about_listView, TYPE_MOVE_UP);
    }
    else if (hold_pad[PAD_DOWN] || hold2_pad[PAD_LEFT_ANALOG_DOWN])
    {
        ListViewMoveTopPos(about_listView, TYPE_MOVE_DOWN);
    }
    else if (hold_pad[PAD_LEFT])
    {
        ListViewMoveTopPos(about_listView, TYPE_MOVE_LEFT);
    }
    else if (hold_pad[PAD_RIGHT])
    {
        ListViewMoveTopPos(about_listView, TYPE_MOVE_RIGHT);
    }

    if (released_pad[PAD_CANCEL])
    {
        GUI_ExitActivity(&about_activity);
    }
}
