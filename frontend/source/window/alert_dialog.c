#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "list/string_list.h"
#include "gui/gui.h"
#include "alert_dialog.h"
#include "utils.h"
#include "utils_string.h"
#include "lang.h"

typedef enum AlertDialogCategoryType
{
    TYPE_ALERT_DIALOG_CATEGORY_MESSAGE,
    TYPE_ALERT_DIALOG_CATEGORY_ITEMS,
} AlertDialogCategoryType;

typedef enum AlertDialogStatusType
{
    TYPE_ALERT_DIALOG_STATUS_HIDE,
    TYPE_ALERT_DIALOG_STATUS_SHOW,
    TYPE_ALERT_DIALOG_STATUS_DISMISS,
} AlertDialogStatusType;

struct AlertDialog
{
    int dont_free;
    GUI_Window *window;
    AlertDialogCategoryType category;
    AlertDialogStatusType status;
    char *title;
    LinkedList *items_list;
    int focus_pos;
    int gradual_count;
    int listview_w;
    int listview_h;
    int listview_wrap_w;
    int listview_wrap_h;
    int listview_scroll_y;
    int dialog_x;
    int dialog_y;
    int dialog_w;
    int dialog_h;
    char *positive_name;
    char *negative_name;
    char *neutral_name;
    int (*onPositiveClick)(AlertDialog *dialog, int which);
    int (*onNegativeClick)(AlertDialog *dialog, int which);
    int (*onNeutralClick)(AlertDialog *dialog, int which);
    void (*freeUserData)(void *data);
    void *userdata;
};

#define DIALOG_PADDING_L 0
#define DIALOG_PADDING_T 0

#define DIALOG_MAX_WIDTH(win_w) (win_w * 0.7f)
#define DIALOG_MAX_HEIGHT(win_h) (win_h * 0.8f)
#define DIALOG_MIN_WIDTH(win_w) (win_w * 0.5f)

#define TITLE_VIEW_PADDING_L 16
#define TITLE_VIEW_PADDING_T 10
#define TITLE_VIEW_HEIGHT (GUI_GetLineHeight() + TITLE_VIEW_PADDING_T * 2)

#define BUTTON_VIEW_CHILD_MARGIN 10
#define BUTTON_VIEW_PADDING_L 16
#define BUTTON_VIEW_PADDING_T 10
#define BUTTON_VIEW_HEIGHT (GUI_GetLineHeight() + BUTTON_VIEW_PADDING_T * 2)

#define MSG_LISTVIEW_PADDING_L 6
#define MSG_LISTVIEW_PADDING_T 6
#define MSG_ITEMVIEW_PADDING_L 10
#define MSG_ITEMVIEW_PADDING_T 6
#define MSG_ITEMVIEW_HEIGHT (GUI_GetLineHeight() + MSG_ITEMVIEW_PADDING_T * 2)
#define MSG_LISTVIEW_MIN_HEIGHT (MSG_ITEMVIEW_HEIGHT * 3 + MSG_LISTVIEW_PADDING_T * 2) // 设置一个最小高度

#define ITEMS_LISTVIEW_PADDING_L 6
#define ITEMS_LISTVIEW_PADDING_T 6
#define ITEMS_ITEMVIEW_PADDING_L 10
#define ITEMS_ITEMVIEW_PADDING_T 6
#define ITEMS_ITEMVIEW_HEIGHT (GUI_GetLineHeight() + ITEMS_ITEMVIEW_PADDING_T * 2)
#define ITEMS_LISTVIEW_MIN_HEIGHT (ITEMS_ITEMVIEW_HEIGHT * 3 + ITEMS_LISTVIEW_PADDING_T * 2) // 设置一个最小高度

#define WINDOW_COLOR_BG COLOR_ALPHA(COLOR_BLACK, 0x7F)
#define DIALOG_COLOR_BG 0xFF5F380A
#define DIALOG_COLOR_BORDER 0xFF6F480A
#define TITLE_COLOR_TEXT COLOR_SPRING_GREEN
#define BUTTON_COLOR_TEXT COLOR_WHITE
#define BUTTON_COLOR_KEY COLOR_SPRING_GREEN
#define ITEMS_COLOR_TEXT COLOR_WHITE
#define ITEMS_COLOR_FOCUS_BG COLOR_ALPHA(COLOR_ORANGE, 0xDF)

#define MAX_DIALOG_GRADUAL_COUNT 10

static int getGradualSize(int size, int gradual, int max)
{
    return (float)size * ((float)gradual / (float)max);
}

static uint32_t getGradualColor(uint32_t color, int gradual, int max)
{
    uint32_t rgb = color & 0x00FFFFFF;
    uint8_t a = color >> 24;
    a = a * ((float)gradual / (float)max);
    return (rgb | (a << 24));
}

static int AlertDialog_SetFocusPos(AlertDialog *dialog, int focus_pos)
{
    if (!dialog)
        return -1;

    int itemviews_h, itemviews_wrap_h;
    int itemview_h;
    int scroll_y; // 相对的滚动偏移，不是真实的，需要起点加上它

    if (focus_pos > LinkedListGetLength(dialog->items_list) - 1)
        focus_pos = LinkedListGetLength(dialog->items_list) - 1;
    if (focus_pos < 0)
        focus_pos = 0;

    if (dialog->category == TYPE_ALERT_DIALOG_CATEGORY_MESSAGE)
    {
        itemviews_h = dialog->listview_h - MSG_LISTVIEW_PADDING_T * 2;
        itemviews_wrap_h = dialog->listview_wrap_h - MSG_LISTVIEW_PADDING_T * 2;
        itemview_h = MSG_ITEMVIEW_HEIGHT;
        scroll_y = 0 - itemview_h * focus_pos; // 顶部对齐
    }
    else
    {
        itemviews_h = dialog->listview_h - ITEMS_LISTVIEW_PADDING_T * 2;
        itemviews_wrap_h = dialog->listview_wrap_h - ITEMS_LISTVIEW_PADDING_T * 2;
        itemview_h = ITEMS_ITEMVIEW_HEIGHT;
        scroll_y = 0 - itemview_h * focus_pos;          // 先顶部对齐
        scroll_y += (itemviews_h / 2 - itemview_h / 2); // 再设置居中
    }

    // 修正scroll_y
    int max_srcoll_y = 0;                                      // 顶部不能越进
    int min_scroll_y = MIN(itemviews_h - itemviews_wrap_h, 0); // 底部不能越进，如果列表总显示长度大于可显示长度的话
    if (scroll_y < min_scroll_y)
        scroll_y = min_scroll_y;
    if (scroll_y > max_srcoll_y)
        scroll_y = max_srcoll_y;

    dialog->listview_scroll_y = scroll_y;

    if (dialog->category == TYPE_ALERT_DIALOG_CATEGORY_MESSAGE) // 显示message时，focus_pos不能超过top_pos，否则按上时focus_pos在动，但列表不会滚动
        dialog->focus_pos = (0 - scroll_y) / itemview_h;        // 获取top_pos，将其设置为focus_pos
    else
        dialog->focus_pos = focus_pos;

    return 0;
}

static int AlertDialog_UpdateLayout(AlertDialog *dialog)
{
    if (!dialog)
        return -1;

    int window_x = 0, window_y = 0;
    int window_w = 0, window_h = 0;
    GUI_GetWindowLayoutPosition(&window_x, &window_y);
    GUI_GetWindowAvailableSize(&window_w, &window_h);

    int dialog_x, dialog_y;
    int dialog_max_w = DIALOG_MAX_WIDTH(window_w);
    int dialog_max_h = DIALOG_MAX_HEIGHT(window_h);
    int dialog_min_w = DIALOG_MIN_WIDTH(window_w);
    int dialog_w = 0;
    int dialog_h = 0;
    int title_h = 0;
    int button_h = 0;
    int listview_w;
    int listview_h;

    if (dialog->title)
        title_h = TITLE_VIEW_HEIGHT;

    if (dialog->positive_name || dialog->negative_name || dialog->neutral_name)
        button_h = BUTTON_VIEW_HEIGHT;

    listview_w = MAX(dialog->listview_wrap_w, dialog_min_w - DIALOG_PADDING_L * 2);

    if (dialog->category == TYPE_ALERT_DIALOG_CATEGORY_MESSAGE)
        listview_h = MAX(dialog->listview_wrap_h, MSG_LISTVIEW_MIN_HEIGHT);
    else
        listview_h = MAX(dialog->listview_wrap_h, ITEMS_LISTVIEW_MIN_HEIGHT);

    dialog_w = listview_w + DIALOG_PADDING_L * 2;
    if (dialog_w > dialog_max_w)
        dialog_w = dialog_max_w;

    dialog_h = title_h + button_h + listview_h + DIALOG_PADDING_T * 2;
    if (dialog_h > dialog_max_h)
        dialog_h = dialog_max_h;

    dialog_x = window_x + (window_w - dialog_w) / 2;
    dialog_y = window_y + (window_h - dialog_h) / 2;

    dialog->dialog_x = dialog_x;
    dialog->dialog_y = dialog_y;
    dialog->dialog_w = dialog_w;
    dialog->dialog_h = dialog_h;
    dialog->listview_w = listview_w;
    dialog->listview_h = dialog_h - title_h - button_h;

    AlertDialog_SetFocusPos(dialog, 0);

    return 0;
}

static int onOpenWindow(GUI_Window *window)
{
    AlertDialog *dialog = (AlertDialog *)GUI_GetWindowData(window);
    if (!dialog)
        return -1;

    dialog->status = TYPE_ALERT_DIALOG_STATUS_SHOW;

    return 0;
}

static int onCloseWindow(GUI_Window *window)
{
    AlertDialog *dialog = (AlertDialog *)GUI_GetWindowData(window);
    if (dialog)
    {
        dialog->window = NULL;           // 设为NULL防止AlertDialog_Destroy时重复关闭window
        AlertDialog_Destroy(dialog);     // 销毁AlertDialog
        GUI_SetWindowData(window, NULL); // 设为NULL以避免可能出现的再次销毁已被销毁的dialog的bug
    }

    return 0;
}

static int onDrawWindow(GUI_Window *window)
{
    AlertDialog *dialog = (AlertDialog *)GUI_GetWindowData(window);
    if (!dialog)
        return -1;

    int window_x = 0, window_y = 0;
    int window_w = 0, window_h = 0;
    GUI_GetWindowLayoutPosition(&window_x, &window_y);
    GUI_GetWindowAvailableSize(&window_w, &window_h);

    int dialog_sx = dialog->dialog_x;
    int dialog_sy = dialog->dialog_y;
    int dialog_w = dialog->dialog_w;
    int dialog_h = dialog->dialog_h;
    int dialog_dx = dialog_sx + dialog_w;
    int dialog_dy = dialog_sy + dialog_h;

    int child_sx = dialog_sx + DIALOG_PADDING_L;
    int child_sy = dialog_sy + DIALOG_PADDING_T;
    int child_dx = dialog_dx - DIALOG_PADDING_L;
    int child_dy = dialog_dy - DIALOG_PADDING_T;
    int child_w = child_dx - child_sx;
    int child_h = child_dy - child_sy;

    int dialog_show_x = dialog_sx;
    int dialog_show_y = dialog_sy;
    int dialog_show_w = dialog_w;
    int dialog_show_h = dialog_h;

    uint32_t window_bg_color = WINDOW_COLOR_BG;
    uint32_t dialog_bg_color = DIALOG_COLOR_BG;
    uint32_t dialog_border_color = DIALOG_COLOR_BORDER;
    uint32_t title_text_color = TITLE_COLOR_TEXT;
    uint32_t button_text_color = BUTTON_COLOR_TEXT;
    uint32_t button_key_color = BUTTON_COLOR_KEY;
    uint32_t items_text_color = ITEMS_COLOR_TEXT;
    uint32_t items_focus_bg_color = ITEMS_COLOR_FOCUS_BG;

    // 设置颜色渐变和窗口大小渐变
    if (dialog->gradual_count < MAX_DIALOG_GRADUAL_COUNT)
    {
        window_bg_color = getGradualColor(window_bg_color, dialog->gradual_count, MAX_DIALOG_GRADUAL_COUNT);
        dialog_bg_color = getGradualColor(dialog_bg_color, dialog->gradual_count, MAX_DIALOG_GRADUAL_COUNT);
        dialog_border_color = getGradualColor(dialog_border_color, dialog->gradual_count, MAX_DIALOG_GRADUAL_COUNT);
        if (dialog->title)
            title_text_color = getGradualColor(title_text_color, dialog->gradual_count, MAX_DIALOG_GRADUAL_COUNT);
        if (dialog->positive_name || dialog->negative_name || dialog->neutral_name)
        {
            button_text_color = getGradualColor(button_text_color, dialog->gradual_count, MAX_DIALOG_GRADUAL_COUNT);
            button_key_color = getGradualColor(button_key_color, dialog->gradual_count, MAX_DIALOG_GRADUAL_COUNT);
        }
        if (dialog->items_list)
        {
            items_text_color = getGradualColor(items_text_color, dialog->gradual_count, MAX_DIALOG_GRADUAL_COUNT);
            items_focus_bg_color = getGradualColor(items_focus_bg_color, dialog->gradual_count, MAX_DIALOG_GRADUAL_COUNT);
        }

        dialog_show_w = getGradualSize(dialog_show_w, dialog->gradual_count, MAX_DIALOG_GRADUAL_COUNT);
        dialog_show_h = getGradualSize(dialog_show_h, dialog->gradual_count, MAX_DIALOG_GRADUAL_COUNT);
        dialog_show_x = dialog_sx + (dialog_w - dialog_show_w) / 2;
        dialog_show_y = dialog_sy + (dialog_h - dialog_show_h) / 2;
    }

    // Draw window bg
    GUI_DrawFillRectangle(window_x, window_y, window_w, window_h, window_bg_color);

    // Set dialog clip
    GUI_SetClipping(dialog_show_x, dialog_show_y, dialog_show_w, dialog_show_h);

    // Draw dialog bg && border
    GUI_DrawFillRectangle(dialog_sx, dialog_sy, dialog_w, dialog_h, dialog_bg_color);
    GUI_DrawEmptyRectangle(dialog_sx, dialog_sy, dialog_w, dialog_h, 1, dialog_border_color);

    // Set childs clip
    GUI_SetClipping(child_sx, child_sy, child_w, child_h);

    int child_x = child_sx;
    int child_y = child_sy;

    // Draw title
    if (dialog->title)
    {
        int view_x = child_x;
        int view_y = child_y;
        int view_w = child_w;
        int view_h = TITLE_VIEW_HEIGHT;
        int text_x = view_x + TITLE_VIEW_PADDING_L;
        int text_y = view_y + TITLE_VIEW_PADDING_T;
        int text_w = view_w - TITLE_VIEW_PADDING_L * 2;
        int text_h = view_h - TITLE_VIEW_PADDING_T;

        GUI_DrawFillRectangle(view_x, view_y + view_h - 1, view_w, 1, dialog_border_color); // Draw divier
        GUI_SetClipping(text_x, text_y, text_w, text_h);
        GUI_DrawText(text_x, text_y, title_text_color, dialog->title);
        GUI_UnsetClipping();

        child_y += view_h;
    }

    // Draw items
    if (dialog->items_list && dialog->listview_h > 0)
    {
        int listview_x = child_x;
        int listview_y = child_y;
        int listview_w = dialog->listview_w;
        int listview_h = dialog->listview_h;

        int itemviews_sx, itemviews_sy;
        int itemviews_dy;
        int itemviews_w, itemviews_h;
        int itemviews_wrap_h;
        int itemview_w, itemview_h;

        if (dialog->category == TYPE_ALERT_DIALOG_CATEGORY_MESSAGE)
        {
            itemviews_sx = listview_x + MSG_LISTVIEW_PADDING_L;
            itemviews_sy = listview_y + MSG_LISTVIEW_PADDING_T;
            itemviews_w = listview_w - MSG_LISTVIEW_PADDING_L * 2;
            itemviews_h = listview_h - MSG_LISTVIEW_PADDING_T * 2;
            itemviews_wrap_h = dialog->listview_wrap_h - MSG_LISTVIEW_PADDING_T * 2;
            itemview_w = itemviews_w;
            itemview_h = MSG_ITEMVIEW_HEIGHT;
        }
        else
        {
            itemviews_sx = listview_x + ITEMS_LISTVIEW_PADDING_L;
            itemviews_sy = listview_y + ITEMS_LISTVIEW_PADDING_T;
            itemviews_w = listview_w - ITEMS_LISTVIEW_PADDING_L * 2;
            itemviews_h = listview_h - ITEMS_LISTVIEW_PADDING_T * 2;
            itemviews_wrap_h = dialog->listview_wrap_h - ITEMS_LISTVIEW_PADDING_T * 2;
            itemview_w = itemviews_w;
            itemview_h = ITEMS_ITEMVIEW_HEIGHT;
        }
        itemviews_dy = itemviews_sy + itemviews_h;

        // Set listview clip
        GUI_SetClipping(listview_x, listview_y, listview_w, listview_h);
        // Set itemviews clip
        GUI_SetClipping(itemviews_sx, itemviews_sy, itemviews_w, itemviews_h);

        LinkedListEntry *entry = LinkedListHead(dialog->items_list);
        const char *text = NULL;
        int itemview_x = itemviews_sx;
        int itemview_y = itemviews_sy + dialog->listview_scroll_y;
        int text_x, text_y;
        int text_w, text_h;
        int n = 0;

        while (entry)
        {
            if (itemview_y + itemview_h < itemviews_sy)
                goto NEXT;
            if (itemview_y > itemviews_dy)
                break;

            if (dialog->category == TYPE_ALERT_DIALOG_CATEGORY_MESSAGE)
            {
                text_x = itemview_x + MSG_ITEMVIEW_PADDING_L;
                text_y = itemview_y + MSG_ITEMVIEW_PADDING_T;
                text_w = itemview_w - MSG_ITEMVIEW_PADDING_L * 2;
                text_h = itemview_h - MSG_ITEMVIEW_PADDING_T;
            }
            else
            {
                text_x = itemview_x + ITEMS_ITEMVIEW_PADDING_L;
                text_y = itemview_y + ITEMS_ITEMVIEW_PADDING_T;
                text_w = itemview_w - ITEMS_ITEMVIEW_PADDING_L * 2;
                text_h = itemview_h - ITEMS_ITEMVIEW_PADDING_T;

                // 绘制focus光标
                if (n == dialog->focus_pos)
                    GUI_DrawFillRectangle(itemview_x, itemview_y, itemview_w, itemview_h, items_focus_bg_color);
            }

            text = (const char *)LinkedListGetEntryData(entry);
            if (text)
            {
                GUI_SetClipping(text_x, text_y, text_w, text_h);
                GUI_DrawText(text_x, text_y, items_text_color, text);
                GUI_UnsetClipping();
            }

        NEXT:
            entry = LinkedListNext(entry);
            itemview_y += itemview_h;
            n++;
        }

        // Unset itemviews clip
        GUI_UnsetClipping(); // listView有padding，我们将scrollbar显示到listView处，这里需要先解除itemviews的clipping

        // Draw scrollbar
        int track_x = listview_x + listview_w - GUI_DEF_SCROLLBAR_SIZE;
        int track_y = listview_y;
        int track_h = listview_h;
        GUI_DrawVerticalScrollbar(track_x, track_y, track_h, itemviews_wrap_h, itemviews_h, 0 - dialog->listview_scroll_y, 0);

        // Unset listview clip
        GUI_UnsetClipping();

        child_y += listview_h;
    }

    // Draw button
    if (dialog->positive_name || dialog->negative_name || dialog->neutral_name)
    {
        int view_x = child_x;
        int view_y = child_y;
        int view_w = child_w;
        int view_h = BUTTON_VIEW_HEIGHT;
        int text_x = view_x + BUTTON_VIEW_PADDING_L;
        int text_y = view_y + BUTTON_VIEW_PADDING_T;
        int text_w = view_w - BUTTON_VIEW_PADDING_L * 2;
        int text_h = view_h - BUTTON_VIEW_PADDING_T;

        GUI_DrawFillRectangle(view_x, view_y, view_w, 1, dialog_border_color); // Draw margin line

        GUI_SetClipping(text_x, text_y, text_w, text_h);

        char buf[24];
        int x = text_x + text_w;
        int y = text_y;

        if (dialog->positive_name)
        {
            x -= GUI_GetTextWidth(dialog->positive_name);
            GUI_DrawText(x, y, button_text_color, dialog->positive_name);
            snprintf(buf, 24, "%s:", cur_lang[LANG_BUTTON_ENTER]);
            x -= GUI_GetTextWidth(buf);
            GUI_DrawText(x, y, button_key_color, buf);
            x -= BUTTON_VIEW_CHILD_MARGIN;
        }
        if (dialog->neutral_name)
        {
            x -= GUI_GetTextWidth(dialog->neutral_name);
            GUI_DrawText(x, y, button_text_color, dialog->neutral_name);
            snprintf(buf, 24, "%s:", cur_lang[LANG_BUTTON_TRIANGLE]);
            x -= GUI_GetTextWidth(buf);
            GUI_DrawText(x, y, button_key_color, buf);
            x -= BUTTON_VIEW_CHILD_MARGIN;
        }
        if (dialog->negative_name)
        {
            x -= GUI_GetTextWidth(dialog->negative_name);
            GUI_DrawText(x, y, button_text_color, dialog->negative_name);
            snprintf(buf, 24, "%s:", cur_lang[LANG_BUTTON_CANCEL]);
            x -= GUI_GetTextWidth(buf);
            GUI_DrawText(x, y, button_key_color, buf);
            x -= BUTTON_VIEW_CHILD_MARGIN;
        }

        GUI_UnsetClipping();
    }

    // Unset childs clip
    GUI_UnsetClipping();
    // Unset dialog clip
    GUI_UnsetClipping();

    return 0;
}

static int onCtrlWindow(GUI_Window *window)
{
    AlertDialog *dialog = (AlertDialog *)GUI_GetWindowData(window);
    if (!dialog)
        return -1;

    if (hold_pad[PAD_UP] || hold2_pad[PAD_LEFT_ANALOG_UP])
    {
        if (dialog->focus_pos > 0)
            AlertDialog_SetFocusPos(dialog, --dialog->focus_pos);
    }
    else if (hold_pad[PAD_DOWN] || hold2_pad[PAD_LEFT_ANALOG_DOWN])
    {
        if (dialog->focus_pos < LinkedListGetLength(dialog->items_list) - 1)
            AlertDialog_SetFocusPos(dialog, ++dialog->focus_pos);
    }
    else if (released_pad[PAD_ENTER])
    {
        if (dialog->onPositiveClick)
            dialog->onPositiveClick(dialog, dialog->focus_pos);
    }
    else if (released_pad[PAD_CANCEL] || released_pad[PAD_PSBUTTON])
    {
        if (dialog->onNegativeClick)
            dialog->onNegativeClick(dialog, dialog->focus_pos);
    }
    else if (released_pad[PAD_TRIANGLE])
    {
        if (dialog->onNeutralClick)
            dialog->onNeutralClick(dialog, dialog->focus_pos);
    }

    return 0;
}

static int onEventWindow(GUI_Window *window)
{
    AlertDialog *dialog = (AlertDialog *)GUI_GetWindowData(window);
    if (!dialog)
        return -1;

    if (dialog->status == TYPE_ALERT_DIALOG_STATUS_SHOW)
    {
        if (dialog->gradual_count < MAX_DIALOG_GRADUAL_COUNT)
            dialog->gradual_count++;
    }
    else if (dialog->status == TYPE_ALERT_DIALOG_STATUS_DISMISS)
    {
        if (dialog->gradual_count > 0)
            dialog->gradual_count--;
        else
            GUI_CloseWindow(window);
    }

    return 0;
}

AlertDialog *AlertDialog_Create()
{
    AlertDialog *dialog = calloc(1, sizeof(AlertDialog));
    if (!dialog)
        return NULL;

    return dialog;
}

void AlertDialog_Destroy(AlertDialog *dialog)
{
    if (!dialog)
        return;

    if (dialog->window)
    {
        GUI_SetWindowData(dialog->window, NULL); // 设置为NULL，防止onWindowClose时销毁dialog
        GUI_CloseWindow(dialog->window);         // 关闭窗口
        dialog->window = NULL;
    }

    if (!dialog->dont_free)
    {
        if (dialog->title)
            free(dialog->title);
        if (dialog->positive_name)
            free(dialog->positive_name);
        if (dialog->negative_name)
            free(dialog->negative_name);
        if (dialog->neutral_name)
            free(dialog->neutral_name);
        if (dialog->items_list)
            LinkedListDestroy(dialog->items_list);
        if (dialog->userdata && dialog->freeUserData)
            dialog->freeUserData(dialog->userdata);
        free(dialog);
    }
}

int AlertDialog_Show(AlertDialog *dialog)
{
    if (!dialog)
        return -1;

    // 如果已经打开了窗口，把它关闭
    if (dialog->window)
    {
        GUI_SetWindowData(dialog->window, NULL); // 设为NULL，防止onWindowClose时销毁dialog
        GUI_CloseWindow(dialog->window);
        dialog->window = NULL;
    }

    dialog->status = TYPE_ALERT_DIALOG_STATUS_HIDE;

    // 创建窗口
    dialog->window = GUI_CreateWindow();
    if (!dialog->window)
        return -1;

    GUI_WindowCallbacks callbacks;
    memset(&callbacks, 0, sizeof(GUI_WindowCallbacks));
    callbacks.onOpen = onOpenWindow;
    callbacks.onClose = onCloseWindow;
    callbacks.onDraw = onDrawWindow;
    callbacks.onCtrl = onCtrlWindow;
    callbacks.onEvent = onEventWindow;
    GUI_SetWindowCallbacks(dialog->window, &callbacks);
    GUI_SetWindowData(dialog->window, dialog);

    if (GUI_OpenWindow(dialog->window) < 0)
    {
        GUI_SetWindowData(dialog->window, NULL);
        GUI_DestroyWindow(dialog->window);
        dialog->window = NULL;
        return -1;
    }

    return 0;
}

int AlertDialog_Dismiss(AlertDialog *dialog)
{
    if (!dialog)
        return -1;

    if (dialog->status == TYPE_ALERT_DIALOG_STATUS_DISMISS) // 已经在关闭中
        return 0;

    if (dialog->status == TYPE_ALERT_DIALOG_STATUS_SHOW)
        dialog->status = TYPE_ALERT_DIALOG_STATUS_DISMISS; // 设置状态，通过onEventWindow关闭
    else
        AlertDialog_Destroy(dialog); // 直接销毁

    return 0;
}

int AlertDialog_Open(AlertDialog *dialog)
{
    if (!dialog)
        return -1;

    dialog->gradual_count = MAX_DIALOG_GRADUAL_COUNT;
    return AlertDialog_Show(dialog);
}

int AlertDialog_Close(AlertDialog *dialog)
{
    if (!dialog)
        return -1;

    dialog->gradual_count = 0;
    return AlertDialog_Dismiss(dialog);
}

int AlertDialog_OnClickDismiss(AlertDialog *dialog, int which)
{
    return AlertDialog_Dismiss(dialog);
}

int AlertDialog_ShowSimpleDialog(const char *title, const char *message)
{
    AlertDialog *dialog = AlertDialog_Create();
    if (!dialog)
        return -1;

    AlertDialog_SetTitle(dialog, title);
    AlertDialog_SetMessage(dialog, message);
    AlertDialog_SetNegativeButton(dialog, cur_lang[LANG_COLSE], AlertDialog_OnClickDismiss);
    AlertDialog_Show(dialog);

    return 0;
}

int AlertDialog_SetAutoFree(AlertDialog *dialog, int auto_free)
{
    if (!dialog)
        return -1;

    dialog->dont_free = !auto_free;

    return 0;
}

int AlertDialog_SetData(AlertDialog *dialog, void *data)
{
    if (!dialog)
        return -1;

    dialog->userdata = data;

    return 0;
}

int AlertDialog_SetTitle(AlertDialog *dialog, const char *title)
{
    if (!dialog)
        return -1;

    if (dialog->title)
    {
        free(dialog->title);
        dialog->title = NULL;
    }

    if (title)
    {
        dialog->title = malloc(strlen(title) + 1);
        if (!dialog->title)
            return -1;
        strcpy(dialog->title, title);
    }

    AlertDialog_UpdateLayout(dialog);

    return 0;
}

int AlertDialog_SetMessage(AlertDialog *dialog, const char *message)
{
    if (!dialog)
        return -1;

    dialog->category = TYPE_ALERT_DIALOG_CATEGORY_MESSAGE;
    dialog->focus_pos = 0;
    dialog->listview_w = 0;
    dialog->listview_h = 0;
    dialog->listview_wrap_w = 0;
    dialog->listview_wrap_h = 0;

    if (!dialog->items_list)
    {
        dialog->items_list = NewStringList();
        if (!dialog->items_list)
            return -1;
    }
    else
    {
        LinkedListEmpty(dialog->items_list);
    }

    if (!message)
        return 0;

    int window_w = 0;
    GUI_GetWindowAvailableSize(&window_w, NULL);
    int itemview_h = MSG_ITEMVIEW_HEIGHT;
    int dialog_max_w = DIALOG_MAX_WIDTH(window_w);
    int listview_max_w = dialog_max_w - DIALOG_PADDING_L * 2;
    int itemview_max_w = listview_max_w - MSG_LISTVIEW_PADDING_L * 2;
    int text_max_w = itemview_max_w - MSG_ITEMVIEW_PADDING_L * 2;

    int text_w = StringToListByWidth(dialog->items_list, message, text_max_w);

    int itemview_wrap_w = text_w + MSG_ITEMVIEW_PADDING_L * 2;
    int itemview_wrap_h = LinkedListGetLength(dialog->items_list) * itemview_h;
    int listview_wrap_w = itemview_wrap_w + MSG_LISTVIEW_PADDING_L * 2;
    int listview_wrap_h = itemview_wrap_h + MSG_LISTVIEW_PADDING_T * 2;

    dialog->listview_wrap_w = listview_wrap_w;
    dialog->listview_wrap_h = listview_wrap_h;

    AlertDialog_UpdateLayout(dialog);

    return 0;
}

int AlertDialog_SetItems(AlertDialog *dialog, char *const *items, int n_items)
{
    if (!dialog)
        return -1;

    dialog->category = TYPE_ALERT_DIALOG_CATEGORY_ITEMS;
    dialog->focus_pos = 0;
    dialog->listview_w = 0;
    dialog->listview_h = 0;
    dialog->listview_wrap_w = 0;
    dialog->listview_wrap_h = 0;

    if (!dialog->items_list)
    {
        dialog->items_list = NewStringList();
        if (!dialog->items_list)
            return -1;
    }
    else
    {
        LinkedListEmpty(dialog->items_list);
    }

    if (!items || n_items <= 0)
        return 0;

    int window_w = 0;
    GUI_GetWindowAvailableSize(&window_w, NULL);
    int itemview_h = ITEMS_ITEMVIEW_HEIGHT;
    int dialog_max_w = DIALOG_MAX_WIDTH(window_w);
    int listview_max_w = dialog_max_w - DIALOG_PADDING_L * 2;
    int itemview_max_w = listview_max_w - ITEMS_LISTVIEW_PADDING_L * 2;
    int text_max_w = itemview_max_w - ITEMS_ITEMVIEW_PADDING_L * 2;

    int text_w = 0;
    int i;
    for (i = 0; i < n_items; i++)
    {
        if (items[i])
        {
            if (text_w < text_max_w)
            {
                int w = GUI_GetTextWidth(items[i]);
                if (w > text_w)
                    text_w = w;
                if (text_w > text_max_w)
                    text_w = text_max_w;
            }
            StringListAdd(dialog->items_list, items[i]);
        }
    }

    int itemview_wrap_w = text_w + ITEMS_ITEMVIEW_PADDING_L * 2;
    int itemview_wrap_h = LinkedListGetLength(dialog->items_list) * itemview_h;
    int listview_wrap_w = itemview_wrap_w + ITEMS_LISTVIEW_PADDING_L * 2;
    int listview_wrap_h = itemview_wrap_h + ITEMS_LISTVIEW_PADDING_T * 2;

    dialog->listview_wrap_w = listview_wrap_w;
    dialog->listview_wrap_h = listview_wrap_h;

    AlertDialog_UpdateLayout(dialog);

    return 0;
}

int AlertDialog_SetPositiveButton(AlertDialog *dialog, const char *name, int (*onClickListener)(AlertDialog *dialog, int which))
{
    if (!dialog)
        return -1;

    if (dialog->positive_name)
    {
        free(dialog->positive_name);
        dialog->positive_name = NULL;
    }

    if (name)
    {
        dialog->positive_name = malloc(strlen(name) + 1);
        if (dialog->positive_name)
            strcpy(dialog->positive_name, name);
    }

    dialog->onPositiveClick = onClickListener;
    AlertDialog_UpdateLayout(dialog);

    return 0;
}

int AlertDialog_SetNegativeButton(AlertDialog *dialog, const char *name, int (*onClickListener)(AlertDialog *dialog, int which))
{
    if (!dialog)
        return -1;

    if (dialog->negative_name)
    {
        free(dialog->negative_name);
        dialog->negative_name = NULL;
    }

    if (name)
    {
        dialog->negative_name = malloc(strlen(name) + 1);
        if (dialog->negative_name)
            strcpy(dialog->negative_name, name);
    }

    dialog->onNegativeClick = onClickListener;
    AlertDialog_UpdateLayout(dialog);

    return 0;
}

int AlertDialog_SetNeutralButton(AlertDialog *dialog, const char *name, int (*onClickListener)(AlertDialog *dialog, int which))
{
    if (!dialog)
        return -1;

    if (dialog->neutral_name)
    {
        free(dialog->neutral_name);
        dialog->neutral_name = NULL;
    }

    if (name)
    {
        dialog->neutral_name = malloc(strlen(name) + 1);
        if (dialog->neutral_name)
            strcpy(dialog->neutral_name, name);
    }

    dialog->onNeutralClick = onClickListener;
    AlertDialog_UpdateLayout(dialog);

    return 0;
}

int AlertDialog_SetFreeDataCallback(AlertDialog *dialog, void (*freeData)(void *data))
{
    if (!dialog)
        return -1;

    dialog->freeUserData = freeData;

    return 0;
}

void *AlertDialog_GetData(AlertDialog *dialog)
{
    return dialog ? dialog->userdata : NULL;
}
