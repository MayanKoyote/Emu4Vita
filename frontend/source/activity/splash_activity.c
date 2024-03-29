#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <psp2/kernel/threadmgr.h>

#include "list/string_list.h"
#include "activity/activity.h"
#include "gui/gui.h"
#include "utils.h"
#include "config.h"
#include "lang.h"

static int onStartActivity(GUI_Activity *activity);
static int onFinishActivity(GUI_Activity *activity);
static int onBeforeDrawActivity(GUI_Activity *activity);
static int onDrawActivity(GUI_Activity *activity);
static int onCtrlActivity(GUI_Activity *activity);

GUI_Activity splash_activity = {
    LANG_NULL,            // Title
    NULL,                 // Button instructions
    NULL,                 // Wallpaper
    1,                    // Disable draw statusbar
    0,                    // Disable home event
    onStartActivity,      // Start callback
    onFinishActivity,     // Finish callback
    onBeforeDrawActivity, // Before draw callback
    onDrawActivity,       // Draw callback
    NULL,                 // After callback
    onCtrlActivity,       // Ctrl callback
    NULL,                 // Event callback
    NULL,                 // User data
};

#define LISTVIEW_PADDING_L 10
#define LISTVIEW_PADDING_T 10

#define ITEMVIEW_PADDING_L 4
#define ITEMVIEW_PADDING_T 4
#define ITEMVIEW_HEIGHT (GUI_GetLineHeight() + ITEMVIEW_PADDING_T * 2)

#define LISTVIEW_COLOR_BG COLOR_ALPHA(COLOR_BLACK, 0xCF)
#define TEXT_COLOR COLOR_WHITE

#define LISTVIEW_PADDING_L 10
#define LISTVIEW_PADDING_T 10

#define ITEMVIEW_PADDING_L 4
#define ITEMVIEW_PADDING_T 4
#define ITEMVIEW_HEIGHT (GUI_GetLineHeight() + ITEMVIEW_PADDING_T * 2)

#define LISTVIEW_COLOR_BG COLOR_ALPHA(COLOR_BLACK, 0xCF)
#define TEXT_COLOR COLOR_WHITE

static LinkedList *log_list = NULL;
static int log_enabled = 0;
static int ctrl_enabled = 0;

static int listview_n_draw_items = 0;
static int listview_top_pos = 0;

static int listview_x, listview_y;
static int listview_w, listview_h;
static int itemview_w, itemview_h;

static void refreshLayout()
{
    GUI_GetActivityLayoutPosition(&splash_activity, &listview_x, &listview_y);
    GUI_GetActivityAvailableSize(&splash_activity, &listview_w, &listview_h);

    itemview_w = listview_w - LISTVIEW_PADDING_L * 2;
    itemview_h = ITEMVIEW_HEIGHT;
    listview_n_draw_items = (listview_h - LISTVIEW_PADDING_T * 2) / itemview_h;
}

void Splash_AddLog(const char *text)
{
    if (!log_enabled || !text)
        return;

    GUI_LockDrawMutex();

    if (!log_list)
    {
        log_list = NewStringList();
        if (!log_list)
            goto UNLOCK_EXIT;
    }
    StringListAdd(log_list, text);

UNLOCK_EXIT:
    GUI_UnlockDrawMutex();
}

void Splash_AddLogf(const char *text, ...)
{
    if (!log_enabled || !text)
        return;

    char buf[1024];
    va_list argptr;
    va_start(argptr, (void *)text);
    vsnprintf(buf, sizeof(buf), text, argptr);
    va_end(argptr);

    Splash_AddLog(buf);
}

void Splash_SetLogEnabled(int enabled)
{
    log_enabled = enabled;
}

void Splash_SetCtrlEnabled(int enabled)
{
    splash_activity.disable_home_event = !enabled;
    ctrl_enabled = enabled;
}

void Splash_SetBgTexture(GUI_Texture *texture)
{
    GUI_LockDrawMutex();

    if (splash_activity.wallpaper && splash_activity.wallpaper != GUI_GetImage(ID_GUI_IMAGE_SPLASH))
    {
        GUI_DestroyTexture(splash_activity.wallpaper);
        splash_activity.wallpaper = NULL;
    }
    if (texture)
        splash_activity.wallpaper = texture;
    else
        splash_activity.wallpaper = GUI_GetImage(ID_GUI_IMAGE_SPLASH);

    GUI_UnlockDrawMutex();
}

static int onStartActivity(GUI_Activity *activity)
{
    splash_activity.disable_home_event = ctrl_enabled;
    refreshLayout();

    return 0;
}

static int onFinishActivity(GUI_Activity *activity)
{
    if (log_list)
    {
        LinkedListDestroy(log_list);
        log_list = NULL;
    }
    if (splash_activity.wallpaper && splash_activity.wallpaper != GUI_GetImage(ID_GUI_IMAGE_SPLASH))
    {
        GUI_DestroyTexture(splash_activity.wallpaper);
        splash_activity.wallpaper = NULL;
    }

    return 0;
}

static int onBeforeDrawActivity(GUI_Activity *activity)
{
    if (!ctrl_enabled)
    {
        int length = LinkedListGetLength(log_list);
        listview_top_pos = length;
        RefreshListPos(&listview_top_pos, length, listview_n_draw_items);
    }

    return 0;
}

static int onDrawActivity(GUI_Activity *activity)
{
    if (!log_enabled)
        return 0;

    // Listview bg
    GUI_DrawFillRectangle(listview_x, listview_y, listview_w, listview_h, LISTVIEW_COLOR_BG);

    int length = LinkedListGetLength(log_list);
    LinkedListEntry *entry = LinkedListFindByNum(log_list, listview_top_pos);

    if (entry)
    {
        // Itemview
        int itemview_x = listview_x + LISTVIEW_PADDING_L;
        int itemview_y = listview_y + LISTVIEW_PADDING_T;
        int itemview_max_dy = listview_y + listview_h - LISTVIEW_PADDING_T;
        int x, y;
        int clip_w, clip_h;
        const char *text;

        while (entry)
        {
            if (itemview_y >= itemview_max_dy)
                break;

            text = (char *)LinkedListGetEntryData(entry);
            if (!text)
                continue;

            x = itemview_x + ITEMVIEW_PADDING_L;
            y = itemview_y + ITEMVIEW_PADDING_T;
            clip_w = itemview_w - ITEMVIEW_PADDING_L * 2;
            clip_h = itemview_h;
            if (clip_h > itemview_max_dy - itemview_y)
                clip_h = itemview_max_dy - itemview_y;
            GUI_SetClipping(x, itemview_y, clip_w, clip_h);
            GUI_DrawText(x, y, TEXT_COLOR, text);
            GUI_UnsetClipping();
            itemview_y += itemview_h;
            entry = LinkedListNext(entry);
        }

        // Scrollbar
        int scrollbar_track_x = listview_x + listview_w - GUI_DEF_SCROLLBAR_SIZE - 2;
        int scrollbar_track_y = listview_y + 2;
        int scrollbar_track_height = listview_h - 4;
        GUI_DrawVerticalScrollbar(scrollbar_track_x, scrollbar_track_y, scrollbar_track_height, length, listview_n_draw_items, listview_top_pos, 0);
    }

    return 0;
}

static int onCtrlActivity(GUI_Activity *activity)
{
    if (ctrl_enabled)
    {
        int length = LinkedListGetLength(log_list);

        if (hold_pad[PAD_UP] || hold2_pad[PAD_LEFT_ANALOG_UP])
        {
            MoveListPos(TYPE_MOVE_UP, &listview_top_pos, length, listview_n_draw_items);
        }
        else if (hold_pad[PAD_DOWN] || hold2_pad[PAD_LEFT_ANALOG_DOWN])
        {
            MoveListPos(TYPE_MOVE_DOWN, &listview_top_pos, length, listview_n_draw_items);
        }
        else if (hold_pad[PAD_LEFT])
        {
            MoveListPos(TYPE_MOVE_LEFT, &listview_top_pos, length, listview_n_draw_items);
        }
        else if (hold_pad[PAD_RIGHT])
        {
            MoveListPos(TYPE_MOVE_RIGHT, &listview_top_pos, length, listview_n_draw_items);
        }
        else if (released_pad[PAD_CANCEL])
        {
            GUI_FinishActivity(activity);
        }
    }

    return 0;
}
