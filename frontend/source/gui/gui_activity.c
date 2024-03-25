#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <psp2/power.h>
#include <psp2/rtc.h>

#include "list/linked_list.h"
#include "gui.h"
#include "init.h"
#include "lang.h"
#include "utils.h"

// Status bar
#define STATUS_BAR_PADDING_T 10
#define STATUS_BAR_PADDING_L 10
#define STATUS_BAR_WIDTH GUI_SCREEN_WIDTH
#define STATUS_BAR_HEIGHT (GUI_GetLineHeight() + STATUS_BAR_PADDING_T * 2)

static LinkedList *activity_list = NULL;

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
    if (!is_vitatv_model)
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
    GetDateString(date_string, date_format, &time);

    char time_string[16];
    GetTimeString(time_string, time_format, &time);

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

int GUI_StartActivity(GUI_Activity *activity)
{
    int ret = 0;
    GUI_LockDrawMutex();

    if (!activity_list)
    {
        activity_list = NewActivityList();
        if (!activity_list)
            goto FAILED;
    }

    if (!LinkedListAdd(activity_list, activity))
        goto FAILED;

    Activity_Start(activity);

EXIT:
    GUI_UnlockDrawMutex();
    return ret;

FAILED:
    ret = -1;
    goto EXIT;
}

int GUI_FinishActivity(GUI_Activity *activity)
{
    GUI_LockDrawMutex();

    LinkedListEntry *entry = LinkedListFindByData(activity_list, activity);
    LinkedListRemove(activity_list, entry);

    GUI_UnlockDrawMutex();
    return 0;
}

int GUI_BeforeDrawActivity()
{
    return Activity_BeforeDraw(GUI_GetCurrentActivity());
}

int GUI_DrawActivity()
{
    GUI_Activity *activity = GUI_GetCurrentActivity();
    if (!activity)
        return -1;

    if (activity->wallpaper)
        Activity_DrawWallpaper(activity->wallpaper);

    Activity_Draw(activity);

    if (!activity->no_statusbar)
    {
        Activity_DrawTopStatusBar(cur_lang[activity->title]);
        Activity_DrawBottomStatusBar(activity->button_instructions);
    }

    return 0;
}

int GUI_AfterDrawActivity()
{
    return Activity_AfterDraw(GUI_GetCurrentActivity());
}

int GUI_CtrlActivity()
{
    return Activity_Ctrl(GUI_GetCurrentActivity());
}

int GUI_EventActivity()
{
    return Activity_Event(GUI_GetCurrentActivity());
}

int GUI_BackToMainActivity()
{
    if (!activity_list)
        return -1;

    LinkedListEntry *head = LinkedListHead(activity_list);
    if (!head)
        return -1;

    LinkedListEntry *entry = LinkedListTail(activity_list);
    while (entry && entry != head)
    {
        LinkedListEntry *prev = LinkedListPrev(entry);
        LinkedListRemove(activity_list, entry);
        entry = prev;
    }

    return 0;
}

int GUI_IsInMainActivity()
{
    return LinkedListGetLength(activity_list) == 1;
}

int GUI_IsHomeEventEnabled()
{
    GUI_Activity *activity = GUI_GetCurrentActivity();
    return activity ? !activity->disable_home_event : 0;
}

int GUI_GetActivityCount()
{
    return LinkedListGetLength(activity_list);
}

GUI_Activity *GUI_GetCurrentActivity()
{
    LinkedListEntry *tail = LinkedListTail(activity_list);

    return (GUI_Activity *)LinkedListGetEntryData(tail);
}

GUI_Activity *GUI_GetPrevActivity(GUI_Activity *activity)
{
    LinkedListEntry *entry = LinkedListFindByData(activity_list, activity);
    LinkedListEntry *prev = LinkedListPrev(entry);

    return (GUI_Activity *)LinkedListGetEntryData(prev);
}

GUI_Activity *GUI_GetNextActivity(GUI_Activity *activity)
{
    LinkedListEntry *entry = LinkedListFindByData(activity_list, activity);
    LinkedListEntry *next = LinkedListNext(entry);

    return (GUI_Activity *)LinkedListGetEntryData(next);
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
