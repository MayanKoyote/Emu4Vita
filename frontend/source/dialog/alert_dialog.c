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

#define STATEBAR_PADDING_L 16
#define STATEBAR_PADDING_T 10

#define TIP_LISTVIEW_PADDING_L 6
#define TIP_LISTVIEW_PADDING_T 12
#define TIP_ITEMVIEW_PADDING_L 10
#define TIP_ITEMVIEW_PADDING_T 4

#define MENU_LISTVIEW_PADDING_L 6
#define MENU_LISTVIEW_PADDING_T 6
#define MENU_ITEMVIEW_PADDING_L 10
#define MENU_ITEMVIEW_PADDING_T 6

#define STATEBAR_HEIGHT (GUI_GetLineHeight() + STATEBAR_PADDING_T * 2)
#define TIP_ITEMVIEW_HEIGHT (GUI_GetLineHeight() + TIP_ITEMVIEW_PADDING_T * 2)
#define MENU_ITEMVIEW_HEIGHT (GUI_GetLineHeight() + MENU_ITEMVIEW_PADDING_T * 2)

#define DIALOG_MAX_WIDTH (GUI_SCREEN_WIDTH * 0.7f)
#define DIALOG_MAX_HEIGHT (GUI_SCREEN_HEIGHT * 0.8f)
#define DIALOG_MIN_WIDTH (GUI_SCREEN_WIDTH * 0.5f)
#define TIP_DIALOG_MIN_HEIGHT (TIP_ITEMVIEW_HEIGHT * 4 + TIP_LISTVIEW_PADDING_T * 2)
#define MENU_DIALOG_MIN_HEIGHT (MENU_ITEMVIEW_HEIGHT * 4 + MENU_LISTVIEW_PADDING_T * 2)

#define OVERLAY_COLOR COLOR_ALPHA(COLOR_BLACK, 0x7F)
#define DIALOG_COLOR_BORDER 0xFF6F480A
#define DIALOG_COLOR_BG 0xFF5F380A
#define ITEMVIEW_COLOR_FOCUS_BG COLOR_ALPHA(COLOR_ORANGE, 0xDF)
#define DIALOG_COLOR_TEXT COLOR_WHITE
#define DIALOG_COLOR_TITLE COLOR_SPRING_GREEN

#define MAX_DIALOG_GRADUAL_COUNT 10

static void drawDialogCallback(GUI_Dialog *dialog);
static void ctrlDialogCallback(GUI_Dialog *dialog);
static int openDialogCallback(GUI_Dialog *dialog);
static int closeDialogCallback(GUI_Dialog *dialog);

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

GUI_Dialog *AlertDialog_Create()
{
    GUI_Dialog *dialog = malloc(sizeof(GUI_Dialog));
    if (!dialog)
        return NULL;

    memset(dialog, 0, sizeof(GUI_Dialog));
    dialog->type = TYPE_GUI_DIALOG_ALERT;
    dialog->drawCallback = drawDialogCallback;
    dialog->ctrlCallBack = ctrlDialogCallback;
    dialog->openCallback = openDialogCallback;
    dialog->closeCallback = closeDialogCallback;

    AlertDialogData *data = malloc(sizeof(AlertDialogData));
    if (!data)
        goto FAILED;
    dialog->userdata = data;

    memset(data, 0, sizeof(AlertDialogData));
    data->auto_free = 1;
    data->dialog_width = DIALOG_MIN_WIDTH;
    data->dialog_height = TIP_DIALOG_MIN_HEIGHT;
    data->dialog = dialog;
    data->list = NewStringList();
    if (!data->list)
        goto FAILED;

    return dialog;

FAILED:
    if (dialog)
        AlertDialog_Destroy(dialog);
    return NULL;
}

void AlertDialog_Destroy(GUI_Dialog *dialog)
{
    if (!dialog || dialog->type != TYPE_GUI_DIALOG_ALERT)
        return;

    if (dialog->userdata)
    {
        AlertDialogData *data = (AlertDialogData *)dialog->userdata;
        if (data->freeCallback)
            data->freeCallback(dialog);
        if (data->title)
            free(data->title);
        if (data->positive_text)
            free(data->positive_text);
        if (data->negative_text)
            free(data->negative_text);
        if (data->neutral_text)
            free(data->neutral_text);
        if (data->list)
            LinkedListDestroy(data->list);
        free(data);
    }

    free(dialog);
}

void AlertDialog_SetAutoFree(GUI_Dialog *dialog, int auto_free)
{
    if (!dialog || dialog->type != TYPE_GUI_DIALOG_ALERT || !dialog->userdata)
        return;

    AlertDialogData *data = (AlertDialogData *)dialog->userdata;
    data->auto_free = auto_free;
}

void AlertDialog_SetUserdata(GUI_Dialog *dialog, void *userdata)
{
    if (!dialog || dialog->type != TYPE_GUI_DIALOG_ALERT || !dialog->userdata)
        return;

    AlertDialogData *data = (AlertDialogData *)dialog->userdata;
    data->userdata = userdata;
}

void AlertDialog_SetTitle(GUI_Dialog *dialog, char *title)
{
    if (!dialog || dialog->type != TYPE_GUI_DIALOG_ALERT || !dialog->userdata)
        return;

    AlertDialogData *data = (AlertDialogData *)dialog->userdata;
    if (data->title)
    {
        free(data->title);
        data->title = NULL;
        data->dialog_height -= STATEBAR_HEIGHT;
    }
    if (title)
    {
        data->title = malloc(strlen(title) + 1);
        if (data->title)
        {
            strcpy(data->title, title);
            data->dialog_height += STATEBAR_HEIGHT;
        }
    }
}

void AlertDialog_SetMessage(GUI_Dialog *dialog, char *message)
{
    if (!dialog || dialog->type != TYPE_GUI_DIALOG_ALERT || !dialog->userdata)
        return;

    AlertDialogData *data = (AlertDialogData *)dialog->userdata;
    data->type = TYPE_ALERT_DIALOG_TIP;
    data->dialog_width = DIALOG_MIN_WIDTH;
    data->dialog_height = TIP_DIALOG_MIN_HEIGHT;
    LinkedListEmpty(data->list);

    if (!message)
        return;

    int max_listview_width = DIALOG_MAX_WIDTH;
    int min_listview_width = DIALOG_MIN_WIDTH;
    int limit_width = max_listview_width - TIP_LISTVIEW_PADDING_L * 2 - TIP_ITEMVIEW_PADDING_L * 2;
    int message_width = StringToListByWidth(data->list, message, limit_width);
    int listview_width = message_width + TIP_ITEMVIEW_PADDING_L * 2 + TIP_LISTVIEW_PADDING_L * 2;
    if (listview_width > max_listview_width)
        listview_width = max_listview_width;
    else if (listview_width < min_listview_width)
        listview_width = min_listview_width;

    int l_length = LinkedListGetLength(data->list);
    int item_height = TIP_ITEMVIEW_HEIGHT;
    int max_listview_height = DIALOG_MAX_HEIGHT - STATEBAR_HEIGHT * 2;
    int min_listview_height = TIP_DIALOG_MIN_HEIGHT;
    int listview_height = l_length * item_height + TIP_LISTVIEW_PADDING_T * 2;
    if (listview_height > max_listview_height)
        listview_height = max_listview_height;
    else if (listview_height < min_listview_height)
        listview_height = min_listview_height;

    data->listview_n_draw_items = (listview_height - TIP_LISTVIEW_PADDING_T * 2) / item_height;

    data->dialog_width = listview_width;
    data->dialog_height = listview_height;
    if (data->title)
        data->dialog_height += STATEBAR_HEIGHT;
    if (data->positive_text || data->negative_text || data->neutral_text)
        data->dialog_height += STATEBAR_HEIGHT;
}

void AlertDialog_SetItems(GUI_Dialog *dialog, char **items, int n_items)
{
    if (!dialog || dialog->type != TYPE_GUI_DIALOG_ALERT || !dialog->userdata)
        return;

    AlertDialogData *data = (AlertDialogData *)dialog->userdata;
    data->type = TYPE_ALERT_DIALOG_MENU;
    data->dialog_width = DIALOG_MIN_WIDTH;
    data->dialog_height = MENU_DIALOG_MIN_HEIGHT;
    LinkedListEmpty(data->list);

    if (!items)
        return;

    char *string;
    int max_listview_width = DIALOG_MAX_WIDTH;
    int min_listview_width = DIALOG_MIN_WIDTH;
    int max_width = max_listview_width - MENU_LISTVIEW_PADDING_L * 2 - MENU_ITEMVIEW_PADDING_L * 2;
    int width = min_listview_width - MENU_LISTVIEW_PADDING_L * 2 - MENU_ITEMVIEW_PADDING_L * 2;
    int i;
    for (i = 0; i < n_items; i++)
    {
        if (width < max_width)
        {
            int w = GUI_GetTextWidth(items[i]);
            if (w > width)
                width = w;
            if (width > max_width)
                width = max_width;
        }
        string = malloc(strlen(items[i]) + 1);
        if (string)
        {
            strcpy(string, items[i]);
            LinkedListAdd(data->list, string);
        }
    }

    int listview_width = width + MENU_ITEMVIEW_PADDING_L * 2 + MENU_LISTVIEW_PADDING_L * 2;

    int l_length = LinkedListGetLength(data->list);
    int item_height = MENU_ITEMVIEW_HEIGHT;
    int max_listview_height = DIALOG_MAX_HEIGHT - STATEBAR_HEIGHT * 2;
    int min_listview_height = MENU_DIALOG_MIN_HEIGHT;
    int listview_height = l_length * item_height + MENU_LISTVIEW_PADDING_T * 2;
    if (listview_height > max_listview_height)
        listview_height = max_listview_height;
    else if (listview_height < min_listview_height)
        listview_height = min_listview_height;

    data->listview_n_draw_items = (listview_height - MENU_LISTVIEW_PADDING_T * 2) / item_height;

    data->dialog_width = listview_width;
    data->dialog_height = listview_height;
    if (data->title)
        data->dialog_height += STATEBAR_HEIGHT;
    if (data->positive_text || data->negative_text || data->neutral_text)
        data->dialog_height += STATEBAR_HEIGHT;
}

void AlertDialog_SetPositiveButton(GUI_Dialog *dialog, char *name, int (*callback)(GUI_Dialog *dialog))
{
    if (!dialog || dialog->type != TYPE_GUI_DIALOG_ALERT || !dialog->userdata)
        return;

    AlertDialogData *data = (AlertDialogData *)dialog->userdata;
    if (data->positive_text || data->negative_text || data->neutral_text)
        data->dialog_height -= STATEBAR_HEIGHT;
    if (data->positive_text)
    {
        free(data->positive_text);
        data->positive_text = NULL;
    }
    if (name)
    {
        data->positive_text = malloc(strlen(name) + 1);
        if (data->positive_text)
            strcpy(data->positive_text, name);
    }
    data->positiveCallback = callback;
    if (data->positive_text || data->negative_text || data->neutral_text)
        data->dialog_height += STATEBAR_HEIGHT;
}

void AlertDialog_SetNegativeButton(GUI_Dialog *dialog, char *name, int (*callback)(GUI_Dialog *dialog))
{
    if (!dialog || dialog->type != TYPE_GUI_DIALOG_ALERT || !dialog->userdata)
        return;

    AlertDialogData *data = (AlertDialogData *)dialog->userdata;
    if (data->positive_text || data->negative_text || data->neutral_text)
        data->dialog_height -= STATEBAR_HEIGHT;
    if (data->negative_text)
    {
        free(data->negative_text);
        data->negative_text = NULL;
    }
    if (name)
    {
        data->negative_text = malloc(strlen(name) + 1);
        if (data->negative_text)
            strcpy(data->negative_text, name);
    }
    data->negativeCallback = callback;
    if (data->positive_text || data->negative_text || data->neutral_text)
        data->dialog_height += STATEBAR_HEIGHT;
}

void AlertDialog_setNeutralButton(GUI_Dialog *dialog, char *name, int (*callback)(GUI_Dialog *dialog))
{
    if (!dialog || dialog->type != TYPE_GUI_DIALOG_ALERT || !dialog->userdata)
        return;

    AlertDialogData *data = (AlertDialogData *)dialog->userdata;
    if (data->positive_text || data->negative_text || data->neutral_text)
        data->dialog_height -= STATEBAR_HEIGHT;
    if (data->neutral_text)
    {
        free(data->neutral_text);
        data->neutral_text = NULL;
    }
    if (name)
    {
        data->neutral_text = malloc(strlen(name) + 1);
        if (data->neutral_text)
            strcpy(data->neutral_text, name);
    }
    data->neutralCallback = callback;
    if (data->positive_text || data->negative_text || data->neutral_text)
        data->dialog_height += STATEBAR_HEIGHT;
}

void AlertDialog_SetFreeCallback(GUI_Dialog *dialog, int (*callback)(GUI_Dialog *dialog))
{
    if (!dialog || dialog->type != TYPE_GUI_DIALOG_ALERT || !dialog->userdata)
        return;

    AlertDialogData *data = (AlertDialogData *)dialog->userdata;
    data->freeCallback = callback;
}

static void drawDialogCallback(GUI_Dialog *dialog)
{
    AlertDialogData *data = (AlertDialogData *)dialog->userdata;

    int dialog_w = data->dialog_width;
    int dialog_h = data->dialog_height;
    int dialog_x = (GUI_SCREEN_WIDTH - dialog_w) / 2;
    int dialog_y = (GUI_SCREEN_HEIGHT - dialog_h) / 2;

    int statebar_w = dialog_w;
    int statebar_h = STATEBAR_HEIGHT;

    int listview_w = dialog_w;
    int listview_h = dialog_h;
    int listview_x = dialog_x;
    int listview_y = dialog_y;
    if (data->title)
    {
        listview_y += statebar_h;
        listview_h -= statebar_h;
    }
    if (data->positive_text || data->negative_text || data->neutral_text)
        listview_h -= statebar_h;

    int top_bar_x = dialog_x;
    int top_bar_y = dialog_y;
    int bottom_bar_x = dialog_x;
    int bottom_bar_y = dialog_y + dialog_h - statebar_h;

    if (data->status == TYPE_DIALOG_STATUS_SHOW)
    {
        if (data->gradual_count < MAX_DIALOG_GRADUAL_COUNT)
        {
            data->gradual_count++;
        }
    }
    else
    {
        if (data->gradual_count > 0)
        {
            data->gradual_count--;
        }
        else
        {
            GUI_CloseDialog(dialog);
            return;
        }
    }

    uint32_t overlay_color = getGradualColor(OVERLAY_COLOR, data->gradual_count, MAX_DIALOG_GRADUAL_COUNT);
    uint32_t dialog_color = getGradualColor(DIALOG_COLOR_BG, data->gradual_count, MAX_DIALOG_GRADUAL_COUNT);
    uint32_t border_color = getGradualColor(DIALOG_COLOR_BORDER, data->gradual_count, MAX_DIALOG_GRADUAL_COUNT);
    uint32_t text_color = getGradualColor(DIALOG_COLOR_TEXT, data->gradual_count, MAX_DIALOG_GRADUAL_COUNT);
    uint32_t title_color = getGradualColor(DIALOG_COLOR_TITLE, data->gradual_count, MAX_DIALOG_GRADUAL_COUNT);
    uint32_t focus_color = getGradualColor(ITEMVIEW_COLOR_FOCUS_BG, data->gradual_count, MAX_DIALOG_GRADUAL_COUNT);

    int x, y;
    int clip_w, clip_h;

    // Draw overlay
    GUI_DrawFillRectangle(0, 0, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT, overlay_color);

    // Set dialog clip
    clip_w = getGradualSize(dialog_w, data->gradual_count, MAX_DIALOG_GRADUAL_COUNT);
    clip_h = getGradualSize(dialog_h, data->gradual_count, MAX_DIALOG_GRADUAL_COUNT);
    GUI_SetClipping((GUI_SCREEN_WIDTH - clip_w) / 2, (GUI_SCREEN_HEIGHT - clip_h) / 2, clip_w, clip_h);

    // Draw dialog bg
    GUI_DrawFillRectangle(dialog_x, dialog_y, dialog_w, dialog_h, dialog_color);
    GUI_DrawEmptyRectangle(dialog_x, dialog_y, dialog_w, dialog_h, 1, border_color);

    // Draw top bar
    if (data->title)
    {
        GUI_DrawFillRectangle(top_bar_x, top_bar_y + statebar_h - 1, statebar_w, 1, border_color); // Draw top bar margin line

        x = top_bar_x + STATEBAR_PADDING_L;
        y = top_bar_y + STATEBAR_PADDING_T;
        clip_w = statebar_w - STATEBAR_PADDING_L * 2;
        clip_h = statebar_h;
        GUI_SetClipping(x, top_bar_y, clip_w, clip_h);
        GUI_DrawText(x, y, title_color, data->title);
        GUI_UnsetClipping();
    }

    // Draw list
    LinkedListEntry *entry = LinkedListFindByNum(data->list, data->top_pos);
    if (entry)
    {
        int listview_padding_l, listview_padding_t;
        int itemview_padding_l, itemview_padding_t;
        int itemview_h;

        if (data->type == TYPE_ALERT_DIALOG_MENU)
        {
            listview_padding_l = MENU_LISTVIEW_PADDING_L;
            listview_padding_t = MENU_LISTVIEW_PADDING_T;
            itemview_padding_l = MENU_ITEMVIEW_PADDING_L;
            itemview_padding_t = MENU_ITEMVIEW_PADDING_T;
            itemview_h = MENU_ITEMVIEW_HEIGHT;
        }
        else
        {
            listview_padding_l = TIP_LISTVIEW_PADDING_L;
            listview_padding_t = TIP_LISTVIEW_PADDING_T;
            itemview_padding_l = TIP_ITEMVIEW_PADDING_L;
            itemview_padding_t = TIP_ITEMVIEW_PADDING_T;
            itemview_h = TIP_ITEMVIEW_HEIGHT;
        }
        int max_itemview_dy = listview_y + listview_h - listview_padding_t;
        int itemview_x = listview_x + listview_padding_l;
        int itemview_y = listview_y + listview_padding_t;
        int itemview_w = listview_w - listview_padding_l * 2;
        int l_length = LinkedListGetLength(data->list);
        const char *text;

        int i;
        for (i = data->top_pos; i < l_length && entry; i++)
        {
            if (itemview_y >= max_itemview_dy)
                break;

            text = (char *)LinkedListGetEntryData(entry);
            if (!text)
                continue;

            clip_w = itemview_w;
            clip_h = itemview_h;
            if (clip_h > max_itemview_dy - itemview_y)
                clip_h = max_itemview_dy - itemview_y;

            GUI_SetClipping(itemview_x, itemview_y, clip_w, clip_h);
            if (data->type == TYPE_ALERT_DIALOG_MENU && i == data->focus_pos)
                GUI_DrawFillRectangle(itemview_x, itemview_y, clip_w, clip_h, focus_color);
            GUI_UnsetClipping();

            x = itemview_x + itemview_padding_l;
            y = itemview_y + itemview_padding_t;
            clip_w = itemview_w - itemview_padding_l * 2;
            GUI_SetClipping(x, itemview_y, clip_w, clip_h);
            GUI_DrawText(x, y, text_color, text);
            GUI_UnsetClipping();

            itemview_y += itemview_h;
            entry = LinkedListNext(entry);
        }

        // Draw scrollbar
        int track_x = listview_x + listview_w - GUI_DEF_SCROLLBAR_SIZE - 2;
        int track_y = listview_y + 2;
        int track_h = listview_h - 4;
        GUI_DrawVerticalScrollbar(track_x, track_y, track_h, l_length, data->listview_n_draw_items, data->top_pos, 0);
    }

    // Draw bottom bar
    if (data->positive_text || data->negative_text || data->neutral_text)
    {
        GUI_DrawFillRectangle(bottom_bar_x, bottom_bar_y, statebar_w, 1, border_color); // Draw bottom bar margin line

        char buf[24];
        x = bottom_bar_x + statebar_w - STATEBAR_PADDING_L;
        y = bottom_bar_y + STATEBAR_PADDING_T;
        if (data->positive_text)
        {
            x -= GUI_GetTextWidth(data->positive_text);
            GUI_DrawText(x, y, text_color, data->positive_text);
            snprintf(buf, 24, "%s:", cur_lang[LANG_BUTTON_ENTER]);
            x -= GUI_GetTextWidth(buf);
            GUI_DrawText(x, y, title_color, buf);
            x -= STATEBAR_PADDING_L;
        }
        if (data->neutral_text)
        {
            x -= GUI_GetTextWidth(data->neutral_text);
            GUI_DrawText(x, y, text_color, data->neutral_text);
            snprintf(buf, 24, "%s:", cur_lang[LANG_BUTTON_TRIANGLE]);
            x -= GUI_GetTextWidth(buf);
            GUI_DrawText(x, y, title_color, buf);
            x -= STATEBAR_PADDING_L;
        }
        if (data->negative_text)
        {
            x -= GUI_GetTextWidth(data->negative_text);
            GUI_DrawText(x, y, text_color, data->negative_text);
            snprintf(buf, 24, "%s:", cur_lang[LANG_BUTTON_CANCEL]);
            x -= GUI_GetTextWidth(buf);
            GUI_DrawText(x, y, title_color, buf);
            x -= STATEBAR_PADDING_L;
        }
    }

    GUI_UnsetClipping();
}

static void ctrlDialogCallback(GUI_Dialog *dialog)
{
    AlertDialogData *data = (AlertDialogData *)dialog->userdata;
    int l_length = LinkedListGetLength(data->list);

    if (hold_pad[PAD_UP] || hold_pad[PAD_LEFT_ANALOG_UP])
    {
        if (data->type == TYPE_ALERT_DIALOG_MENU)
            MoveListPos(TYPE_MOVE_UP, &data->top_pos, &data->focus_pos, l_length, data->listview_n_draw_items);
        else
            MoveListPosNoFocus(TYPE_MOVE_UP, &data->top_pos, l_length, data->listview_n_draw_items);
    }
    else if (hold_pad[PAD_DOWN] || hold_pad[PAD_LEFT_ANALOG_DOWN])
    {
        if (data->type == TYPE_ALERT_DIALOG_MENU)
            MoveListPos(TYPE_MOVE_DOWN, &data->top_pos, &data->focus_pos, l_length, data->listview_n_draw_items);
        else
            MoveListPosNoFocus(TYPE_MOVE_DOWN, &data->top_pos, l_length, data->listview_n_draw_items);
    }

    if (released_pad[PAD_ENTER])
    {
        if (data->positiveCallback)
            data->positiveCallback(dialog);
    }
    else if (released_pad[PAD_CANCEL])
    {
        if (data->negativeCallback)
            data->negativeCallback(dialog);
    }
    else if (released_pad[PAD_TRIANGLE])
    {
        if (data->neutralCallback)
            data->neutralCallback(dialog);
    }
}

static int openDialogCallback(GUI_Dialog *dialog)
{
    if (!dialog || !dialog->userdata)
        return -1;

    AlertDialogData *data = (AlertDialogData *)dialog->userdata;
    data->opened = 1;
    data->status = TYPE_DIALOG_STATUS_SHOW;
    data->gradual_count = 0;

    return 0;
}

static int closeDialogCallback(GUI_Dialog *dialog)
{
    if (!dialog || !dialog->userdata)
        return -1;

    AlertDialogData *data = (AlertDialogData *)dialog->userdata;
    data->opened = 0;
    data->status = TYPE_DIALOG_STATUS_DISMISS;
    if (data->auto_free)
        AlertDialog_Destroy(dialog);

    return 0;
}

int AlertDialog_ShowSimpleTipDialog(char *title, char *message)
{
    GUI_Dialog *dialog = AlertDialog_Create();
    if (!dialog)
        return -1;
    AlertDialog_SetTitle(dialog, title);
    AlertDialog_SetMessage(dialog, message);
    AlertDialog_SetNegativeButton(dialog, cur_lang[LANG_COLSE], AlertDialog_Dismiss);
    AlertDialog_Show(dialog);
    return 0;
}

int AlertDialog_Show(GUI_Dialog *dialog)
{
    if (!dialog || dialog->type != TYPE_GUI_DIALOG_ALERT || !dialog->userdata)
        return -1;

    AlertDialogData *data = (AlertDialogData *)dialog->userdata;
    if (!data->opened)
        GUI_OpenDialog(dialog);
    else
        data->status = TYPE_DIALOG_STATUS_SHOW;

    return 0;
}

int AlertDialog_Dismiss(GUI_Dialog *dialog)
{
    if (!dialog || dialog->type != TYPE_GUI_DIALOG_ALERT || !dialog->userdata)
        return -1;

    AlertDialogData *data = (AlertDialogData *)dialog->userdata;
    data->status = TYPE_DIALOG_STATUS_DISMISS;

    return 0;
}
