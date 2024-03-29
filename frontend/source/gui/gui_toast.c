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
    LinkedListEntry *entry;
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

static SceKernelLwMutexWork gui_toast_mutex = {0};
static LinkedList *gui_toast_list = NULL;

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

static int Toast_BeforeDraw(GUI_Toast *toast)
{
    if (!toast)
        return -1;

    uint64_t cur_micros = sceKernelGetProcessTimeWide();
    if (toast->status == TYPE_TOAST_STATUS_SHOW)
    {
        if (cur_micros >= toast->finish_show_micros)
            toast->status = TYPE_TOAST_STATUS_DISMISS;
    }
    else if (toast->status == TYPE_TOAST_STATUS_DISMISS)
    {
        if (cur_micros - toast->finish_show_micros >= MAX_TOAST_GRADUAL_MICROS)
        {
            if (!LinkedListRemove(gui_toast_list, toast->entry))
                Toast_Destroy(toast);
        }
    }
    else
    {
        toast->start_show_micros = sceKernelGetProcessTimeWide();
        toast->finish_show_micros = toast->start_show_micros + MAX_TOAST_GRADUAL_MICROS + toast->show_micros;
        toast->status = TYPE_TOAST_STATUS_SHOW;
    }

    return 0;
}

static int Toast_Draw(GUI_Toast *toast)
{
    if (!toast)
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
            bg_color = 0;
            text_color = 0;
        }
    }

    TextViewSetBgColor(toast->textView, bg_color);
    TextViewSetTextColor(toast->textView, text_color);
    LayoutParamsDraw(toast->textView);

    return 0;
}

static GUI_Toast *Toast_GetCurrent()
{
    return (GUI_Toast *)LinkedListGetEntryData(LinkedListHead(gui_toast_list));
}

int GUI_BeforeDrawToast()
{
    sceKernelLockLwMutex(&gui_toast_mutex, 1, NULL);
    int ret = Toast_BeforeDraw(Toast_GetCurrent());
    sceKernelUnlockLwMutex(&gui_toast_mutex, 1);
    return ret;
}

int GUI_DrawToast()
{
    sceKernelLockLwMutex(&gui_toast_mutex, 1, NULL);
    int ret = Toast_Draw(Toast_GetCurrent());
    sceKernelUnlockLwMutex(&gui_toast_mutex, 1);
    return ret;
}

int GUI_AfterDrawToast()
{
    return 0;
}

int GUI_ShowToast(const char *message, float second)
{
    int ret = 0;

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
    sceKernelLockLwMutex(&gui_toast_mutex, 1, NULL);
    toast->entry = LinkedListAdd(gui_toast_list, toast);
    if (!toast->entry)
    {
        ret = -1;
        Toast_Destroy(toast);
    }
    sceKernelUnlockLwMutex(&gui_toast_mutex, 1);

    return ret;
}

int GUI_InitToast()
{
    gui_toast_list = NewToastList();
    if (!gui_toast_list)
        return -1;

    sceKernelCreateLwMutex(&gui_toast_mutex, "gui_toast_mutex", 2, 0, NULL);

    return 0;
}

int GUI_DeinitToast()
{
    sceKernelLockLwMutex(&gui_toast_mutex, 1, NULL);
    LinkedListDestroy(gui_toast_list);
    gui_toast_list = NULL;
    sceKernelUnlockLwMutex(&gui_toast_mutex, 1);
    sceKernelDeleteLwMutex(&gui_toast_mutex);

    return 0;
}
