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
static int onDrawActivity(GUI_Activity *activity);
static int onCtrlActivity(GUI_Activity *activity);

GUI_Activity splash_activity = {
    LANG_NULL,        // Title
    NULL,             // Button instructions
    NULL,             // Wallpaper
    1,                // Disable draw statusbar
    onStartActivity,  // Start callback
    onFinishActivity, // Finish callback
    onDrawActivity,   // Draw callback
    onCtrlActivity,   // Ctrl callback
    NULL,             // Event callback
    NULL,             // User data
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

static SceKernelLwMutexWork splash_mutex;
static LinkedList *log_list = NULL;
static int log_enabled = 0;

static int listview_auto_scroll = 0;
static int listview_n_draw_items = 0;
static int listview_top_pos = 0;

static int layout_x, layout_y;
static int available_w, available_h;

static int listview_x, listview_y;
static int listview_w, listview_h;
static int itemview_w, itemview_h;
static int listview_n_draw_items;

static int scrollbar_track_x, scrollbar_track_y;
static int scrollbar_track_height;

static void refreshLayout()
{
    GUI_GetActivityLayoutPosition(&splash_activity, &layout_x, &layout_y);
    GUI_GetActivityAvailableSize(&splash_activity, &available_w, &available_h);

    listview_x = layout_x;
    listview_y = layout_y;
    listview_w = available_w;
    listview_h = available_h;

    itemview_w = listview_w - LISTVIEW_PADDING_L * 2;
    itemview_h = ITEMVIEW_HEIGHT;
    listview_n_draw_items = (listview_h - LISTVIEW_PADDING_T * 2) / itemview_h;

    scrollbar_track_x = listview_x + listview_w - GUI_DEF_SCROLLBAR_SIZE - 2;
    scrollbar_track_y = listview_y + 2;
    scrollbar_track_height = listview_h - 4;
}

void Splash_AddLog(const char *text)
{
    if (!log_enabled || !text)
        return;

    sceKernelLockLwMutex(&splash_mutex, 1, NULL);

    char *string = malloc(strlen(text) + 1);
    if (string)
    {
        strcpy(string, text);
        LinkedListAdd(log_list, string);
    }

    sceKernelUnlockLwMutex(&splash_mutex, 1);
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

void Splash_SetListviewAutoScroll(int enable)
{
    listview_auto_scroll = enable;
}

void Splash_SetBgTexture(GUI_Texture *texture)
{
    if (splash_activity.wallpaper && splash_activity.wallpaper != GUI_GetDefaultSplash())
    {
        GUI_DestroyTexture(splash_activity.wallpaper);
        splash_activity.wallpaper = NULL;
    }
    if (texture)
        splash_activity.wallpaper = texture;
    else
        splash_activity.wallpaper = GUI_GetDefaultSplash();
}

void Splash_SetLogEnabled(int enabled)
{
    log_enabled = enabled;
}

static int onStartActivity(GUI_Activity *activity)
{
    refreshLayout();

    if (log_list)
        LinkedListDestroy(log_list);
    log_list = NewStringList();

    sceKernelCreateLwMutex(&splash_mutex, "splash_mutex", 2, 0, NULL);
    return 0;
}

static int onFinishActivity(GUI_Activity *activity)
{
    sceKernelLockLwMutex(&splash_mutex, 1, NULL);
    if (log_list)
        LinkedListDestroy(log_list);
    log_list = NULL;
    if (splash_activity.wallpaper && splash_activity.wallpaper != GUI_GetDefaultSplash())
    {
        GUI_DestroyTexture(splash_activity.wallpaper);
        splash_activity.wallpaper = NULL;
    }
    sceKernelUnlockLwMutex(&splash_mutex, 1);
    sceKernelDeleteLwMutex(&splash_mutex);
    return 0;
}

static int onDrawActivity(GUI_Activity *activity)
{
    if (!log_enabled || !log_list)
        return 0;

    sceKernelLockLwMutex(&splash_mutex, 1, NULL);

    // Listview bg
    GUI_DrawFillRectangle(listview_x, listview_y, listview_w, listview_h, LISTVIEW_COLOR_BG);

    int l_length = LinkedListGetLength(log_list);
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

        int i;
        for (i = listview_top_pos; i < l_length; i++)
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
        GUI_DrawVerticalScrollbar(scrollbar_track_x, scrollbar_track_y, scrollbar_track_height, l_length, listview_n_draw_items, listview_top_pos, 0);
    }

    sceKernelUnlockLwMutex(&splash_mutex, 1);

    return 0;
}

static int onCtrlActivity(GUI_Activity *activity)
{
    if (released_pad[PAD_CANCEL])
    {
        GUI_FinishActivity(activity);
        return 0;
    }

    if (!log_enabled || !log_list)
        return 0;

    sceKernelLockLwMutex(&splash_mutex, 1, NULL);

    int l_length = LinkedListGetLength(log_list);

    if (listview_auto_scroll)
    {
        listview_top_pos = l_length;
        RefreshListPos(&listview_top_pos, l_length, listview_n_draw_items);
    }
    else
    {
        if (hold_pad[PAD_UP] || hold2_pad[PAD_LEFT_ANALOG_UP])
        {
            MoveListPos(TYPE_MOVE_UP, &listview_top_pos, l_length, listview_n_draw_items);
        }
        else if (hold_pad[PAD_DOWN] || hold2_pad[PAD_LEFT_ANALOG_DOWN])
        {
            MoveListPos(TYPE_MOVE_DOWN, &listview_top_pos, l_length, listview_n_draw_items);
        }
        else if (hold_pad[PAD_LEFT])
        {
            MoveListPos(TYPE_MOVE_LEFT, &listview_top_pos, l_length, listview_n_draw_items);
        }
        else if (hold_pad[PAD_RIGHT])
        {
            MoveListPos(TYPE_MOVE_RIGHT, &listview_top_pos, l_length, listview_n_draw_items);
        }
    }

    sceKernelUnlockLwMutex(&splash_mutex, 1);

    return 0;
}
