#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <psp2/kernel/threadmgr.h>
#include <psp2/power.h>
#include <psp2/rtc.h>

#include "list/linked_list.h"
#include "gui.h"
#include "app.h"
#include "lang.h"
#include "utils.h"

// Status bar
#define STATUS_BAR_PADDING_T 10
#define STATUS_BAR_PADDING_L 10
#define STATUS_BAR_WIDTH GUI_SCREEN_WIDTH
#define STATUS_BAR_HEIGHT (GUI_GetLineHeight() + STATUS_BAR_PADDING_T * 2)

static SceKernelLwMutexWork gui_activity_mutex = {0};
static LinkedList *gui_activity_list = NULL;

static int Activity_Finish(GUI_Activity *activity);

static void freeActivityEntryData(void *data)
{
    if (data)
    {
        GUI_Activity *activity = (GUI_Activity *)data;
        Activity_Finish(activity);
    }
}

static LinkedList *NewActivityList()
{
    LinkedList *list = NewLinkedList();
    if (!list)
        return NULL;

    LinkedListSetFreeEntryDataCallback(list, freeActivityEntryData);

    return list;
}

static int Activity_Start(GUI_Activity *activity)
{
    if (!activity)
        return -1;

    if (activity->onStart)
        activity->onStart(activity);

    return 0;
}

static int Activity_Finish(GUI_Activity *activity)
{
    if (!activity)
        return -1;

    if (activity->onFinish)
        activity->onFinish(activity);

    return 0;
}

static int Activity_BeforeDraw(GUI_Activity *activity)
{
    if (!activity)
        return -1;

    if (activity->onBeforeDraw)
        activity->onBeforeDraw(activity);

    return 0;
}

static int Activity_Draw(GUI_Activity *activity)
{
    if (!activity)
        return -1;

    if (activity->onDraw)
        activity->onDraw(activity);

    return 0;
}

static int Activity_AfterDraw(GUI_Activity *activity)
{
    if (!activity)
        return -1;

    if (activity->onAfterDraw)
        activity->onAfterDraw(activity);

    return 0;
}

static int Activity_Ctrl(GUI_Activity *activity)
{
    if (!activity)
        return -1;

    if (activity->onCtrl)
        activity->onCtrl(activity);

    return 0;
}

static int Activity_Event(GUI_Activity *activity)
{
    if (!activity)
        return -1;

    if (activity->onEvent)
        activity->onEvent(activity);

    return 0;
}

static void Activity_DrawWallpaper(GUI_Texture *texture)
{
    if (texture)
    {
        float x_scale = (float)GUI_SCREEN_WIDTH / (float)GUI_GetTextureWidth(texture);
        float y_scale = (float)GUI_SCREEN_HEIGHT / (float)GUI_GetTextureHeight(texture);
        GUI_DrawTextureScale(texture, 0, 0, x_scale, y_scale);
    }
}

static void Activity_DrawTopStatusBar(char *title)
{
    int statusbar_x = 0;
    int statusbar_y = 0;
    int statusbar_w = STATUS_BAR_WIDTH;
    int statusbar_h = STATUS_BAR_HEIGHT;

    GUI_DrawFillRectangle(statusbar_x, statusbar_y, statusbar_w, statusbar_h, GUI_DEF_COLOR_BG);

    int x, y;
    char string[64];

    x = statusbar_x + STATUS_BAR_PADDING_L;
    y = statusbar_y + STATUS_BAR_PADDING_T;

    GUI_SetClipping(x, y, statusbar_w - STATUS_BAR_PADDING_L * 2, statusbar_h - STATUS_BAR_PADDING_T);

    if (title)
        GUI_DrawText(x, y, COLOR_WHITE, title);

    x = statusbar_x + statusbar_w - STATUS_BAR_PADDING_L;
    if (!IsVitatvModel())
    {
        uint32_t color;
        if (scePowerIsBatteryCharging())
            color = COLOR_YELLOW;
        else if (scePowerIsLowBattery())
            color = COLOR_RED;
        else
            color = COLOR_GREEN;

        int percent = scePowerGetBatteryLifePercent();
        snprintf(string, sizeof(string), "%d%%", percent);

        int battery_x = x - GUI_GetTextWidth(string);
        GUI_DrawText(battery_x, y, color, string);
        x = battery_x - STATUS_BAR_PADDING_L;
    }

    // Date & time
    SceDateTime time;
    sceRtcGetCurrentClock(&time, 0);

    char date_string[24];
    GetDateString(date_string, system_date_format, &time);

    char time_string[16];
    GetTimeString(time_string, system_time_format, &time);

    snprintf(string, sizeof(string), "%s  %s", date_string, time_string);
    int date_time_x = x - GUI_GetTextWidth(string);
    GUI_DrawText(date_time_x, y, GUI_DEF_COLOR_TEXT, string);

    GUI_UnsetClipping();
}

static void Activity_DrawBottomStatusBar(GUI_ButtonInstruction *instructions)
{
    int statusbar_w = STATUS_BAR_WIDTH;
    int statusbar_h = STATUS_BAR_HEIGHT;
    int statusbar_x = 0;
    int statusbar_y = GUI_SCREEN_HEIGHT - statusbar_h;

    GUI_DrawFillRectangle(statusbar_x, statusbar_y, statusbar_w, statusbar_h, GUI_DEF_COLOR_BG);

    if (instructions)
    {
        int x = statusbar_x + STATUS_BAR_PADDING_L;
        int y = statusbar_y + STATUS_BAR_PADDING_T;

        GUI_SetClipping(x, y, statusbar_w - STATUS_BAR_PADDING_L * 2, statusbar_h - STATUS_BAR_PADDING_T);

        int i;
        for (i = 0; instructions[i].button != LANG_NULL; i++)
        {
            if (!instructions[i].visibility)
                continue;
            x += GUI_DrawTextf(x, y, COLOR_AZURE, "%s:", cur_lang[instructions[i].button]);
            x += GUI_DrawText(x, y, COLOR_WHITE, cur_lang[instructions[i].instruction]);
            x += STATUS_BAR_PADDING_L;
        }

        GUI_UnsetClipping();
    }
}

static GUI_Activity *Activity_GetCurrent()
{
    return (GUI_Activity *)LinkedListGetEntryData(LinkedListTail(gui_activity_list));
}

int GUI_StartActivity(GUI_Activity *activity)
{
    if (!activity)
        return -1;

    int ret = 0;
    sceKernelLockLwMutex(&gui_activity_mutex, 1, NULL);
    if (!LinkedListAdd(gui_activity_list, activity))
        ret = -1;
    else
        Activity_Start(activity);
    sceKernelUnlockLwMutex(&gui_activity_mutex, 1);
    return ret;
}

int GUI_FinishActivity(GUI_Activity *activity)
{
    if (!activity)
        return -1;

    sceKernelLockLwMutex(&gui_activity_mutex, 1, NULL);
    LinkedListEntry *entry = LinkedListFindByData(gui_activity_list, activity);
    LinkedListRemove(gui_activity_list, entry);
    sceKernelUnlockLwMutex(&gui_activity_mutex, 1);
    return 0;
}

int GUI_FinishAllActivities()
{
    sceKernelLockLwMutex(&gui_activity_mutex, 1, NULL);
    LinkedListEmpty(gui_activity_list);
    sceKernelUnlockLwMutex(&gui_activity_mutex, 1);
    return 0;
}

int GUI_FinishOtherActivities()
{
    sceKernelLockLwMutex(&gui_activity_mutex, 1, NULL);
    LinkedListEntry *entry = LinkedListPrev(LinkedListTail(gui_activity_list));
    while (entry)
    {
        LinkedListEntry *prev = LinkedListPrev(entry);
        LinkedListRemove(gui_activity_list, entry);
        entry = prev;
    }
    sceKernelUnlockLwMutex(&gui_activity_mutex, 1);
    return 0;
}

int GUI_BeforeDrawActivity()
{
    sceKernelLockLwMutex(&gui_activity_mutex, 1, NULL);
    int ret = Activity_BeforeDraw(Activity_GetCurrent());
    sceKernelUnlockLwMutex(&gui_activity_mutex, 1);
    return ret;
}

int GUI_DrawActivity()
{
    sceKernelLockLwMutex(&gui_activity_mutex, 1, NULL);

    GUI_Activity *activity = Activity_GetCurrent();
    if (activity)
    {
        if (activity->wallpaper)
            Activity_DrawWallpaper(activity->wallpaper);

        Activity_Draw(activity);

        if (!activity->no_statusbar)
        {
            Activity_DrawTopStatusBar(cur_lang[activity->title]);
            Activity_DrawBottomStatusBar(activity->button_instructions);
        }
    }

    sceKernelUnlockLwMutex(&gui_activity_mutex, 1);
    return 0;
}

int GUI_AfterDrawActivity()
{
    sceKernelLockLwMutex(&gui_activity_mutex, 1, NULL);
    int ret = Activity_AfterDraw(Activity_GetCurrent());
    sceKernelUnlockLwMutex(&gui_activity_mutex, 1);
    return ret;
}

int GUI_CtrlActivity()
{
    sceKernelLockLwMutex(&gui_activity_mutex, 1, NULL);
    int ret = Activity_Ctrl(Activity_GetCurrent());
    sceKernelUnlockLwMutex(&gui_activity_mutex, 1);
    return ret;
}

int GUI_EventActivity()
{
    sceKernelLockLwMutex(&gui_activity_mutex, 1, NULL);
    int ret = Activity_Event(Activity_GetCurrent());
    sceKernelUnlockLwMutex(&gui_activity_mutex, 1);
    return ret;
}

int GUI_BackToMainActivity()
{
    sceKernelLockLwMutex(&gui_activity_mutex, 1, NULL);

    LinkedListEntry *entry = LinkedListNext(LinkedListHead(gui_activity_list));
    while (entry)
    {
        LinkedListEntry *next = LinkedListNext(entry);
        LinkedListRemove(gui_activity_list, entry);
        entry = next;
    }

    sceKernelUnlockLwMutex(&gui_activity_mutex, 1);
    return 0;
}

int GUI_IsInMainActivity()
{
    return LinkedListGetLength(gui_activity_list) == 1;
}

int GUI_IsHomeEventEnabled()
{
    sceKernelLockLwMutex(&gui_activity_mutex, 1, NULL);
    GUI_Activity *activity = Activity_GetCurrent();
    int ret = activity ? !activity->disable_home_event : 0;
    sceKernelUnlockLwMutex(&gui_activity_mutex, 1);
    return ret;
}

int GUI_GetActivityCount()
{
    return LinkedListGetLength(gui_activity_list);
}

int GUI_GetActivityLayoutPosition(GUI_Activity *activity, int *x, int *y)
{
    if (x)
        *x = 0;
    if (y)
    {
        if (activity && !activity->no_statusbar)
            *y = STATUS_BAR_HEIGHT;
        else
            *y = 0;
    }
    return 0;
}

int GUI_GetActivityAvailableSize(GUI_Activity *activity, int *w, int *h)
{
    if (w)
        *w = GUI_SCREEN_WIDTH;
    if (h)
    {
        if (activity && !activity->no_statusbar)
            *h = GUI_SCREEN_HEIGHT - STATUS_BAR_HEIGHT * 2;
        else
            *h = GUI_SCREEN_HEIGHT;
    }
    return 0;
}

int GUI_InitActivity()
{
    gui_activity_list = NewActivityList();
    if (!gui_activity_list)
        return -1;

    sceKernelCreateLwMutex(&gui_activity_mutex, "gui_activity_mutex", 2, 0, NULL);

    return 0;
}

int GUI_DeinitActivity()
{
    sceKernelLockLwMutex(&gui_activity_mutex, 1, NULL);
    LinkedListDestroy(gui_activity_list);
    gui_activity_list = NULL;
    sceKernelUnlockLwMutex(&gui_activity_mutex, 1);
    sceKernelDeleteLwMutex(&gui_activity_mutex);

    return 0;
}
