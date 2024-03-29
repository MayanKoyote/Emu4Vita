#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "activity/activity.h"
#include "gui/gui.h"
#include "utils.h"
#include "lang.h"

#include "about_text.c"

static int onStartActivity(GUI_Activity *activity);
static int onFinishActivity(GUI_Activity *activity);
static int onDrawActivity(GUI_Activity *activity);
static int onCtrlActivity(GUI_Activity *activity);

static GUI_ButtonInstruction button_instructions[] = {
    {LANG_LOCAL_BUTTON_CANCEL, LANG_BACK, 1},
    {LANG_NULL, LANG_NULL, 1},
};

GUI_Activity about_activity = {
    LANG_ABOUT_TITLE,    // Title
    button_instructions, // Button instructions
    NULL,                // Wallpaper
    0,                   // Disable draw statusbar
    0,                   // Disable home event
    onStartActivity,     // Start callback
    onFinishActivity,    // Finish callback
    NULL,                // Before draw callback
    onDrawActivity,      // Draw callback
    NULL,                // After callback
    onCtrlActivity,      // Ctrl callback
    NULL,                // Event callback
    NULL,                // User data
};

#define LISTVIEW_PADDING_L 6
#define LISTVIEW_PADDING_T 6
#define ITEMVIEW_PADDING_L 4
#define ITEMVIEW_PADDING_T 4

#define TEXT_COLOR COLOR_ORANGE

static Layout *about_layout = NULL;
static ListView *about_listView = NULL;

static int onGetListLength(void *list)
{
    return list ? N_ABOUT_TEXTS : 0;
}

static void *onGetHeadListEntry(void *list)
{
    return list ? ((void **)list)[0] : NULL;
}

static void *onGetNextListEntry(void *list, void *entry, int id)
{
    int next_id = id + 1;
    if (list && next_id >= 0 && next_id <= N_ABOUT_TEXTS - 1)
        return ((void **)list)[next_id];
    return NULL;
}

static void *onCreateItemView(void *data)
{
    ItemView *itemView = NewItemView();
    if (!itemView)
        return NULL;

    LayoutParamsSetPadding(itemView, ITEMVIEW_PADDING_L, ITEMVIEW_PADDING_L, ITEMVIEW_PADDING_T, ITEMVIEW_PADDING_T);
    LayoutParamsSetLayoutSize(itemView, TYPE_LAYOUT_PARAMS_MATH_PARENT, TYPE_LAYOUT_PARAMS_WRAP_CONTENT);
    ItemViewSetNameTextColor(itemView, TEXT_COLOR);
    ItemViewSetNameText(itemView, (const char *)data);

    return itemView;
}

static int onSetItemViewData(void *itemView, void *data)
{
    return ItemViewSetData((ItemView *)itemView, data);
}

static int createLayout()
{
    int layout_x = 0, layout_y = 0;
    int available_w = 0, available_h = 0;

    if (about_layout)
        return 0;

    GUI_GetActivityLayoutPosition(&about_activity, &layout_x, &layout_y);
    GUI_GetActivityAvailableSize(&about_activity, &available_w, &available_h);

    about_layout = NewLayout();
    if (!about_layout)
        return -1;
    LayoutParamsSetOrientation(about_layout, TYPE_LAYOUT_PARAMS_ORIENTATION_VERTICAL);
    LayoutParamsSetLayoutPosition(about_layout, layout_x, layout_y);
    LayoutParamsSetAvailableSize(about_layout, available_w, available_h);
    LayoutParamsSetLayoutSize(about_layout, TYPE_LAYOUT_PARAMS_MATH_PARENT, TYPE_LAYOUT_PARAMS_MATH_PARENT);
    LayoutParamsSetPadding(about_layout, GUI_DEF_MAIN_LAYOUT_PADDING, GUI_DEF_MAIN_LAYOUT_PADDING, GUI_DEF_MAIN_LAYOUT_PADDING, GUI_DEF_MAIN_LAYOUT_PADDING);

    about_listView = NewListView();
    if (!about_listView)
        return -1;
    LayoutParamsSetOrientation(about_listView, TYPE_LAYOUT_PARAMS_ORIENTATION_VERTICAL);
    LayoutParamsSetLayoutSize(about_listView, TYPE_LAYOUT_PARAMS_MATH_PARENT, TYPE_LAYOUT_PARAMS_MATH_PARENT);
    LayoutParamsSetPadding(about_listView, LISTVIEW_PADDING_L, LISTVIEW_PADDING_L, LISTVIEW_PADDING_T, LISTVIEW_PADDING_T);
    ListViewSetBgColor(about_listView, GUI_DEF_COLOR_BG);
    ListViewCallbacks callbacks;
    memset(&callbacks, 0, sizeof(ListViewCallbacks));
    callbacks.onCreateItemView = onCreateItemView;
    callbacks.onSetItemViewData = onSetItemViewData;
    callbacks.onGetListLength = onGetListLength;
    callbacks.onGetHeadListEntry = onGetHeadListEntry;
    callbacks.onGetNextListEntry = onGetNextListEntry;
    ListViewSetList(about_listView, about_texts, &callbacks);
    LayoutAddView(about_layout, about_listView);

    LayoutParamsUpdate(about_layout);

    return 0;
}

static int destroyLayout()
{
    LayoutParamsDestroy(about_layout);
    about_layout = NULL;
    about_listView = NULL;

    return 0;
}

static int onStartActivity(GUI_Activity *activity)
{
    about_activity.wallpaper = GUI_GetImage(ID_GUI_IMAGE_WALLPAPER);
    createLayout();
    return 0;
}

static int onFinishActivity(GUI_Activity *activity)
{
    destroyLayout();
    return 0;
}

static int onDrawActivity(GUI_Activity *activity)
{
    LayoutParamsDraw(about_layout);
    return 0;
}

static int onCtrlActivity(GUI_Activity *activity)
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
    else if (released_pad[PAD_CANCEL])
    {
        GUI_FinishActivity(activity);
    }

    return 0;
}
