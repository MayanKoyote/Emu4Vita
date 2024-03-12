#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "psp2/kernel/processmgr.h"

#include "list/linked_list.h"
#include "gui.h"

typedef enum ToastStatusType
{
    TYPE_TOAST_STATUS_HIDE,
    TYPE_TOAST_STATUS_SHOW,
    TYPE_TOAST_STATUS_DISMISS,
} ToastStatusType;

typedef struct GUI_Toast
{
    ToastStatusType status;
    uint64_t show_micros;
    uint64_t start_show_micros;
    uint64_t finish_show_micros;
    TextView *textView;
} GUI_Toast;

#define MICROS_PER_SECOND 1000000llu

#define TOAST_CHILD_MARGIN 4

#define TOAST_PADDING_L 20
#define TOAST_PADDING_T 20
#define TOAST_MAX_WIDTH (GUI_SCREEN_WIDTH / 2)
#define TOAST_MAX_HEIGHT GUI_SCREEN_HEIGHT

#define TOAST_TEXTVIEW_PADDING_L 6
#define TOAST_TEXTVIEW_PADDING_T 6

#define TOAST_COLOR_BG COLOR_ALPHA(COLOR_BLACK, 0xDF)
#define TOAST_COLOR_TEXT COLOR_WHITE

#define MAX_TOAST_GRADUAL_MICROS 200000llu

static LinkedList *toast_list = NULL;

static void Toast_Destroy(GUI_Toast *toast);

static uint32_t getGradualColor(uint32_t color, uint64_t gradual, uint64_t max)
{
    uint32_t rgb = color & 0x00FFFFFF;
    uint8_t a = color >> 24;
    a = a * ((double)gradual / (double)max);
    return (rgb | (a << 24));
}

static void freeToastEntryData(void *data)
{
    if (data)
    {
        GUI_Toast *toast = (GUI_Toast *)data;
        Toast_Destroy(toast);
    }
}

static LinkedList *NewToastList()
{
    LinkedList *list = NewLinkedList();
    if (!list)
        return NULL;

    LinkedListSetFreeEntryDataCallback(list, freeToastEntryData);

    return list;
}

static GUI_Toast *Toast_Create()
{
    GUI_Toast *toast = (GUI_Toast *)calloc(1, sizeof(GUI_Toast));
    if (!toast)
        return NULL;

    toast->textView = NewTextView();
    if (!toast->textView)
    {
        free(toast);
        return NULL;
    }

    return toast;
}

static void Toast_Destroy(GUI_Toast *toast)
{
    if (toast)
    {
        LayoutParamsDestroy(toast->textView);
        free(toast);
    }
}

static int Toast_Remove(GUI_Toast *toast)
{
    if (!toast_list)
        return 0;

    LinkedListEntry *entry = LinkedListFindByData(toast_list, toast);

    return LinkedListRemove(toast_list, entry);
}

static int Toast_Show(GUI_Toast *toast)
{
    if (!toast)
        return -1;

    toast->start_show_micros = sceKernelGetProcessTimeWide();
    toast->finish_show_micros = toast->start_show_micros + MAX_TOAST_GRADUAL_MICROS + toast->show_micros;
    toast->status = TYPE_TOAST_STATUS_SHOW;

    return 0;
}

static int Toast_Dismiss(GUI_Toast *toast)
{
    if (!toast)
        return -1;

    if (toast->status == TYPE_TOAST_STATUS_DISMISS)
        return 0;

    if (toast->status == TYPE_TOAST_STATUS_SHOW)
        toast->status = TYPE_TOAST_STATUS_DISMISS;
    else if (!Toast_Remove(toast))
        Toast_Destroy(toast);

    return 0;
}

static int Toast_Draw(GUI_Toast *toast)
{
    if (!toast || !toast->textView)
        return -1;

    if (toast->status == TYPE_TOAST_STATUS_HIDE)
        return 0;

    int measured_h = 0;
    LayoutParamsGetMeasuredSize(toast->textView, NULL, &measured_h);
    int layout_x = TOAST_PADDING_L;
    int layout_y = GUI_SCREEN_HEIGHT - TOAST_PADDING_T - measured_h;
    LayoutParamsSetLayoutPosition(toast->textView, layout_x, layout_y);

    uint32_t bg_color = TOAST_COLOR_BG;
    uint32_t text_color = TOAST_COLOR_TEXT;
    uint64_t cur_micros = sceKernelGetProcessTimeWide();
    if (toast->status == TYPE_TOAST_STATUS_SHOW)
    {
        uint64_t showwing_micros = cur_micros - toast->start_show_micros;
        if (showwing_micros < MAX_TOAST_GRADUAL_MICROS)
        {
            bg_color = getGradualColor(bg_color, showwing_micros, MAX_TOAST_GRADUAL_MICROS);
            text_color = getGradualColor(text_color, showwing_micros, MAX_TOAST_GRADUAL_MICROS);
        }
    }
    else if (toast->status == TYPE_TOAST_STATUS_DISMISS)
    {
        uint64_t dismissing_micros = cur_micros - toast->finish_show_micros;
        if (dismissing_micros < MAX_TOAST_GRADUAL_MICROS)
        {
            bg_color = getGradualColor(bg_color, MAX_TOAST_GRADUAL_MICROS - dismissing_micros, MAX_TOAST_GRADUAL_MICROS);
            text_color = getGradualColor(text_color, MAX_TOAST_GRADUAL_MICROS - dismissing_micros, MAX_TOAST_GRADUAL_MICROS);
        }
        else
        {
            return 0;
        }
    }

    TextViewSetBgColor(toast->textView, bg_color);
    TextViewSetTextColor(toast->textView, text_color);
    LayoutParamsDraw(toast->textView);

    return 0;
}

int GUI_DrawToast()
{
    if (!toast_list)
        return -1;

    LinkedListEntry *entry = LinkedListHead(toast_list);
    GUI_Toast *toast = (GUI_Toast *)LinkedListGetEntryData(entry);
    if (!toast)
        return 0;

    Toast_Draw(toast);

    return 0;
}

int GUI_EventToast()
{
    if (!toast_list)
        return -1;

    LinkedListEntry *entry = LinkedListHead(toast_list);
    GUI_Toast *toast = (GUI_Toast *)LinkedListGetEntryData(entry);
    if (!toast)
        return 0;

    uint64_t cur_micros = sceKernelGetProcessTimeWide();
    if (toast->status == TYPE_TOAST_STATUS_SHOW)
    {
        if (cur_micros >= toast->finish_show_micros)
            Toast_Dismiss(toast);
    }
    else if (toast->status == TYPE_TOAST_STATUS_DISMISS)
    {
        if (cur_micros - toast->finish_show_micros >= MAX_TOAST_GRADUAL_MICROS)
            LinkedListRemove(toast_list, entry);
    }
    else
    {
        Toast_Show(toast);
    }

    return 0;
}

int GUI_ShowToast(const char *message, float second)
{
    if (!toast_list)
    {
        toast_list = NewToastList();
        if (!toast_list)
            return -1;
    }

    GUI_Toast *toast = Toast_Create();
    if (!toast)
        return -1;

    // 设置显示时间
    toast->show_micros = second * MICROS_PER_SECOND;

    // 设置TextView
    LayoutParamsSetAvailableSize(toast->textView, TOAST_MAX_WIDTH, TOAST_MAX_HEIGHT);
    LayoutParamsSetLayoutSize(toast->textView, TYPE_LAYOUT_PARAMS_WRAP_CONTENT, TYPE_LAYOUT_PARAMS_WRAP_CONTENT);
    LayoutParamsSetPadding(toast->textView, TOAST_TEXTVIEW_PADDING_L, TOAST_TEXTVIEW_PADDING_L, TOAST_TEXTVIEW_PADDING_T, TOAST_TEXTVIEW_PADDING_T);
    TextViewSetText(toast->textView, message);
    TextViewSetBgColor(toast->textView, TOAST_COLOR_BG);
    TextViewSetTextColor(toast->textView, TOAST_COLOR_TEXT);
    LayoutParamsUpdate(toast->textView);

    // 添加到Toast列表里
    LinkedListAdd(toast_list, toast);

    return 0;
}
