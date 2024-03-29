#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "gui/gui.h"
#include "utils.h"

typedef enum SlidingMenuStatusType
{
    TYPE_SLIDING_MENU_STATUS_HIDE,
    TYPE_SLIDING_MENU_STATUS_SHOW,
    TYPE_SLIDING_MENU_STATUS_DISMISS,
} SlidingMenuStatusType;

struct SlidingMenu
{
    int dont_free;
    GUI_Window *window;
    SlidingMenuModeType mode;
    SlidingMenuChoiceType choice_type;
    SlidingMenuStatusType status;
    char **items;
    int n_items;
    int *selects;
    int focus_pos;
    int gradual_count;
    int listview_w;
    int listview_h;
    int listview_wrap_w;
    int listview_wrap_h;
    int listview_scroll_y;
    int slidingmenu_x;
    int slidingmenu_y;
    int slidingmenu_w;
    int slidingmenu_h;
    int (*onItemClick)(SlidingMenu *slidingMenu, int which);
    int (*onSelectChanged)(SlidingMenu *slidingMenu);
    int (*onClose)(SlidingMenu *slidingMenu);
    void (*freeUserData)(void *data);
    void *userdata;
};

#define SLIDINGMENU_PADDING_L 0
#define SLIDINGMENU_PADDING_T 0

#define ITEMS_LISTVIEW_PADDING_L 10
#define ITEMS_LISTVIEW_PADDING_T 10
#define ITEMS_ITEMVIEW_PADDING_L 10
#define ITEMS_ITEMVIEW_PADDING_T 6
#define ITEMS_CHOICEBOX_MARGIN 20
#define ITEMS_ITEMVIEW_HEIGHT (GUI_GetLineHeight() + ITEMS_ITEMVIEW_PADDING_T * 2)
#define ITEMS_LISTVIEW_MIN_WIDTH (GUI_GetLineHeight() * 10)
#define ITEMS_LISTVIEW_MIN_HEIGHT (ITEMS_ITEMVIEW_HEIGHT * 4)

#define SLIDINGMENU_COLOR_BG COLOR_ALPHA(COLOR_BLACK, 0xDF)
#define ITEMS_COLOR_TEXT COLOR_WHITE
#define ITEMS_COLOR_FOCUS_BG COLOR_ALPHA(COLOR_AZURE, 0xDF)

#define MAX_SLIDINGMENU_GRADUAL_COUNT 10

static int getGradualSize(int size, int gradual, int max)
{
    return (float)size * ((float)gradual / (float)max);
}

static int onOpenWindow(GUI_Window *window)
{
    SlidingMenu *slidingMenu = (SlidingMenu *)GUI_GetWindowData(window);
    if (!slidingMenu)
        return -1;

    slidingMenu->status = TYPE_SLIDING_MENU_STATUS_SHOW;

    return 0;
}

static int SlidingMenu_UpdateLayout(SlidingMenu *slidingMenu)
{
    if (!slidingMenu)
        return -1;

    int window_x = 0, window_y = 0;
    int window_w = 0, window_h = 0;
    GUI_GetWindowLayoutPosition(&window_x, &window_y);
    GUI_GetWindowAvailableSize(&window_w, &window_h);

    int slidingmenu_max_w = window_w;
    int slidingmenu_max_h = window_h;
    int slidingmenu_x = window_x;
    int slidingmenu_y = window_y;
    int slidingmenu_w = slidingmenu_max_w;
    int slidingmenu_h = slidingmenu_max_h;
    int listview_wrap_w = slidingMenu->listview_wrap_w;
    int listview_wrap_h = slidingMenu->listview_wrap_h;

    if (slidingMenu->choice_type != TYPE_SLIDING_MENU_CHOICE_NONE)
    {
        int choicebox_w = ITEMS_ITEMVIEW_HEIGHT - ITEMS_ITEMVIEW_PADDING_T * 2;
        listview_wrap_w += (choicebox_w + ITEMS_CHOICEBOX_MARGIN);
    }

    if (slidingMenu->mode == TYPE_SLIDING_MENU_MODE_TOP) // 从顶部往下弹出
    {
        slidingmenu_max_h = window_h * 2.0f / 3.0f;
        slidingmenu_h = MAX(listview_wrap_h, ITEMS_LISTVIEW_MIN_HEIGHT);
        if (slidingmenu_h > slidingmenu_max_h)
            slidingmenu_h = slidingmenu_max_h;
    }
    else if (slidingMenu->mode == TYPE_SLIDING_MENU_MODE_BOTTOM) // 从底部往上弹出
    {
        slidingmenu_max_h = window_h * 2.0f / 3.0f;
        slidingmenu_h = MAX(listview_wrap_h, ITEMS_LISTVIEW_MIN_HEIGHT);
        if (slidingmenu_h > slidingmenu_max_h)
            slidingmenu_h = slidingmenu_max_h;
        slidingmenu_y = window_y + window_h - slidingmenu_h;
    }
    else if (slidingMenu->mode == TYPE_SLIDING_MENU_MODE_LEFT) // 从左侧往右弹出
    {
        slidingmenu_max_w = window_w * 2.0f / 3.0f;
        slidingmenu_w = MAX(listview_wrap_w, ITEMS_LISTVIEW_MIN_WIDTH);
        if (slidingmenu_w > slidingmenu_max_w)
            slidingmenu_w = slidingmenu_max_w;
    }
    else // 默认从右侧往左弹出
    {
        slidingmenu_max_w = window_w * 2.0f / 3.0f;
        slidingmenu_w = MAX(listview_wrap_w, ITEMS_LISTVIEW_MIN_WIDTH);
        if (slidingmenu_w > slidingmenu_max_w)
            slidingmenu_w = slidingmenu_max_w;
        slidingmenu_x = window_x + window_w - slidingmenu_w;
    }

    slidingMenu->slidingmenu_x = slidingmenu_x;
    slidingMenu->slidingmenu_y = slidingmenu_y;
    slidingMenu->slidingmenu_w = slidingmenu_w;
    slidingMenu->slidingmenu_h = slidingmenu_h;
    slidingMenu->listview_w = slidingmenu_w - SLIDINGMENU_PADDING_L * 2;
    slidingMenu->listview_h = slidingmenu_h - SLIDINGMENU_PADDING_T * 2;

    SlidingMenu_SetFocusPos(slidingMenu, 0);

    return 0;
}

static int onCloseWindow(GUI_Window *window)
{
    SlidingMenu *slidingMenu = (SlidingMenu *)GUI_GetWindowData(window);
    if (slidingMenu)
    {
        slidingMenu->window = NULL;
        GUI_SetWindowData(window, NULL);
        SlidingMenu_Destroy(slidingMenu);
    }

    return 0;
}

static int onBeforeDrawWindow(GUI_Window *window)
{
    SlidingMenu *slidingMenu = (SlidingMenu *)GUI_GetWindowData(window);
    if (!slidingMenu)
        return -1;

    if (slidingMenu->status == TYPE_SLIDING_MENU_STATUS_SHOW)
    {
        if (slidingMenu->gradual_count < MAX_SLIDINGMENU_GRADUAL_COUNT)
            slidingMenu->gradual_count++;
    }
    else if (slidingMenu->status == TYPE_SLIDING_MENU_STATUS_DISMISS)
    {
        if (slidingMenu->gradual_count > 0)
            slidingMenu->gradual_count--;
        else
            GUI_CloseWindow(window);
    }

    return 0;
}

static int onDrawWindow(GUI_Window *window)
{
    SlidingMenu *slidingMenu = (SlidingMenu *)GUI_GetWindowData(window);
    if (!slidingMenu)
        return -1;

    int slidingmenu_sx = slidingMenu->slidingmenu_x;
    int slidingmenu_sy = slidingMenu->slidingmenu_y;
    int slidingmenu_w = slidingMenu->slidingmenu_w;
    int slidingmenu_h = slidingMenu->slidingmenu_h;
    int slidingmenu_dx = slidingmenu_sx + slidingmenu_w;
    int slidingmenu_dy = slidingmenu_sy + slidingmenu_h;

    int slidingmenu_show_x = slidingmenu_sx;
    int slidingmenu_show_y = slidingmenu_sy;
    int slidingmenu_show_w = slidingmenu_w;
    int slidingmenu_show_h = slidingmenu_h;

    // 更新滚动坐标
    if (slidingMenu->gradual_count < MAX_SLIDINGMENU_GRADUAL_COUNT)
    {
        if (slidingMenu->mode == TYPE_SLIDING_MENU_MODE_TOP) // 从顶部往下弹出
        {
            slidingmenu_show_h = getGradualSize(slidingmenu_h, slidingMenu->gradual_count, MAX_SLIDINGMENU_GRADUAL_COUNT);
            slidingmenu_sy -= (slidingmenu_h - slidingmenu_show_h);
        }
        else if (slidingMenu->mode == TYPE_SLIDING_MENU_MODE_BOTTOM) // 从底部往上弹出
        {
            slidingmenu_show_h = getGradualSize(slidingmenu_h, slidingMenu->gradual_count, MAX_SLIDINGMENU_GRADUAL_COUNT);
            slidingmenu_sy += (slidingmenu_h - slidingmenu_show_h);
            slidingmenu_show_y = slidingmenu_sy;
        }
        else if (slidingMenu->mode == TYPE_SLIDING_MENU_MODE_LEFT) // 从左侧往右弹出
        {
            slidingmenu_show_w = getGradualSize(slidingmenu_w, slidingMenu->gradual_count, MAX_SLIDINGMENU_GRADUAL_COUNT);
            slidingmenu_sx -= (slidingmenu_w - slidingmenu_show_w);
        }
        else // 默认从右侧往左弹出
        {
            slidingmenu_show_w = getGradualSize(slidingmenu_w, slidingMenu->gradual_count, MAX_SLIDINGMENU_GRADUAL_COUNT);
            slidingmenu_sx += (slidingmenu_w - slidingmenu_show_w);
            slidingmenu_show_x = slidingmenu_sx;
        }
    }

    int children_sx = slidingmenu_sx + SLIDINGMENU_PADDING_L;
    int children_sy = slidingmenu_sy + SLIDINGMENU_PADDING_T;
    int children_dx = slidingmenu_dx - SLIDINGMENU_PADDING_L;
    int children_dy = slidingmenu_dy - SLIDINGMENU_PADDING_T;
    int children_w = children_dx - children_sx;
    int children_h = children_dy - children_sy;

    // Set slidingMenu clip
    GUI_SetClipping(slidingmenu_show_x, slidingmenu_show_y, slidingmenu_show_w, slidingmenu_show_h);

    // Draw slidingMenu bg
    GUI_DrawFillRectangle(slidingmenu_sx, slidingmenu_sy, slidingmenu_w, slidingmenu_h, SLIDINGMENU_COLOR_BG);

    // Set children clip
    GUI_SetClipping(children_sx, children_sy, children_w, children_h);

    int child_x = children_sx;
    int child_y = children_sy;

    // Draw items
    if (slidingMenu->items)
    {
        int listview_x = child_x;
        int listview_y = child_y;
        int listview_w = slidingMenu->listview_w;
        int listview_h = slidingMenu->listview_h;

        int itemviews_sx = listview_x + ITEMS_LISTVIEW_PADDING_L;
        int itemviews_sy = listview_y + ITEMS_LISTVIEW_PADDING_T;
        int itemviews_w = listview_w - ITEMS_LISTVIEW_PADDING_L * 2;
        int itemviews_h = listview_h - ITEMS_LISTVIEW_PADDING_T * 2;
        int itemviews_dy = itemviews_sy + itemviews_h;
        int itemviews_wrap_h = slidingMenu->listview_wrap_h - ITEMS_LISTVIEW_PADDING_T * 2;
        int itemview_w = itemviews_w;
        int itemview_h = ITEMS_ITEMVIEW_HEIGHT;

        // Set listview clip
        GUI_SetClipping(listview_x, listview_y, listview_w, listview_h);
        // Set itemviews clip
        GUI_SetClipping(itemviews_sx, itemviews_sy, itemviews_w, itemviews_h);

        int itemview_x = itemviews_sx;
        int itemview_y = itemviews_sy + slidingMenu->listview_scroll_y;
        int choicebox_h = itemview_h - ITEMS_ITEMVIEW_PADDING_T * 2;
        int choicebox_w = choicebox_h;
        int choicebox_x, choicebox_y;
        int text_x, text_y;
        int text_w, text_h;

        int i;
        for (i = 0; i < slidingMenu->n_items; i++)
        {
            if (itemview_y + itemview_h < itemviews_sy)
                goto NEXT;
            if (itemview_y > itemviews_dy)
                break;

            // 绘制focus光标
            if (i == slidingMenu->focus_pos)
                GUI_DrawFillRectangle(itemview_x, itemview_y, itemview_w, itemview_h, ITEMS_COLOR_FOCUS_BG);

            // Draw choice box
            if (slidingMenu->choice_type != TYPE_SLIDING_MENU_CHOICE_NONE)
            {
                GUI_Texture *choicebox_texture = NULL;
                GUI_Texture *on_texture = NULL;
                GUI_Texture *off_texture = NULL;
                choicebox_x = itemview_x + itemview_w - ITEMS_ITEMVIEW_PADDING_L - choicebox_w;
                choicebox_y = itemview_y + ITEMS_ITEMVIEW_PADDING_T;

                if (slidingMenu->choice_type == TYPE_SLIDING_MENU_CHOICE_SINGLE)
                {
                    on_texture = GUI_GetImage(ID_GUI_IMAGE_RADIOBUTTON_ON);
                    off_texture = GUI_GetImage(ID_GUI_IMAGE_RADIOBUTTON_OFF);
                }
                else if (slidingMenu->choice_type == TYPE_SLIDING_MENU_CHOICE_MULTIPLE)
                {
                    on_texture = GUI_GetImage(ID_GUI_IMAGE_CHECKBOX_ON);
                    off_texture = GUI_GetImage(ID_GUI_IMAGE_CHECKBOX_OFF);
                }
                if (slidingMenu->selects[i])
                    choicebox_texture = on_texture;
                else
                    choicebox_texture = off_texture;
                if (choicebox_texture)
                {
                    float x_scale = (float)choicebox_w / (float)GUI_GetTextureWidth(choicebox_texture);
                    float y_scale = (float)choicebox_h / (float)GUI_GetTextureHeight(choicebox_texture);
                    GUI_DrawTextureScale(choicebox_texture, choicebox_x, choicebox_y, x_scale, y_scale);
                }

                text_w = itemview_w - ITEMS_ITEMVIEW_PADDING_L * 2 - choicebox_w - ITEMS_CHOICEBOX_MARGIN;
            }
            else
            {
                text_w = itemview_w - ITEMS_ITEMVIEW_PADDING_L * 2;
            }

            if (slidingMenu->items[i])
            {
                text_x = itemview_x + ITEMS_ITEMVIEW_PADDING_L;
                text_y = itemview_y + ITEMS_ITEMVIEW_PADDING_T;
                text_h = itemview_h - ITEMS_ITEMVIEW_PADDING_T;
                GUI_SetClipping(text_x, text_y, text_w, text_h);
                GUI_DrawText(text_x, text_y, ITEMS_COLOR_TEXT, slidingMenu->items[i]);
                GUI_UnsetClipping();
            }

        NEXT:
            itemview_y += itemview_h;
        }

        // Unset itemviews clip
        GUI_UnsetClipping();

        // Draw scrollbar
        int track_x = listview_x + listview_w - GUI_DEF_SCROLLBAR_SIZE;
        int track_y = listview_y;
        int track_h = listview_h;
        GUI_DrawVerticalScrollbar(track_x, track_y, track_h, itemviews_wrap_h, itemviews_h, 0 - slidingMenu->listview_scroll_y, 0);

        // Unset listview clip
        GUI_UnsetClipping();
    }

    // Unset children clip
    GUI_UnsetClipping();
    // Unset slidingMenu clip
    GUI_UnsetClipping();

    return 0;
}

static int onCtrlWindow(GUI_Window *window)
{
    SlidingMenu *slidingMenu = (SlidingMenu *)GUI_GetWindowData(window);
    if (!slidingMenu)
        return -1;

    if (hold_pad[PAD_UP] || hold2_pad[PAD_LEFT_ANALOG_UP])
    {
        if (slidingMenu->focus_pos > 0)
            SlidingMenu_SetFocusPos(slidingMenu, --slidingMenu->focus_pos);
    }
    else if (hold_pad[PAD_DOWN] || hold2_pad[PAD_LEFT_ANALOG_DOWN])
    {
        if (slidingMenu->focus_pos < slidingMenu->n_items - 1)
            SlidingMenu_SetFocusPos(slidingMenu, ++slidingMenu->focus_pos);
    }
    else if (released_pad[PAD_ENTER])
    {
        if (slidingMenu->selects)
        {
            if (slidingMenu->choice_type == TYPE_SLIDING_MENU_CHOICE_SINGLE)
            {
                int i;
                for (i = 0; i < slidingMenu->n_items; i++)
                {
                    if (i == slidingMenu->focus_pos)
                        slidingMenu->selects[i] = 1;
                    else
                        slidingMenu->selects[i] = 0;
                }
            }
            else if (slidingMenu->choice_type == TYPE_SLIDING_MENU_CHOICE_MULTIPLE)
            {
                slidingMenu->selects[slidingMenu->focus_pos] = !slidingMenu->selects[slidingMenu->focus_pos];
            }
        }

        if (slidingMenu->onItemClick)
            slidingMenu->onItemClick(slidingMenu, slidingMenu->focus_pos);
    }
    else if (released_pad[PAD_Y]) // 多选模式下清除所有选中
    {
        if (slidingMenu->selects)
        {
            if (slidingMenu->choice_type == TYPE_SLIDING_MENU_CHOICE_MULTIPLE)
            {
                int i;
                for (i = 0; i < slidingMenu->n_items; i++)
                {
                    slidingMenu->selects[i] = 0;
                }
            }
        }

        if (slidingMenu->onSelectChanged)
            slidingMenu->onSelectChanged(slidingMenu);
    }
    else if (released_pad[PAD_CANCEL])
    {
        SlidingMenu_Dismiss(slidingMenu);
    }

    return 0;
}

SlidingMenu *SlidingMenu_Create()
{
    SlidingMenu *slidingMenu = calloc(1, sizeof(SlidingMenu));
    if (!slidingMenu)
        return NULL;

    return slidingMenu;
}

void SlidingMenu_Destroy(SlidingMenu *slidingMenu)
{
    if (!slidingMenu)
        return;

    if (slidingMenu->window)
    {
        GUI_SetWindowData(slidingMenu->window, NULL); // 设置为NULL，防止onWindowClose时销毁slidingMenu
        GUI_CloseWindow(slidingMenu->window);         // 关闭窗口
        slidingMenu->window = NULL;
    }

    if (!slidingMenu->dont_free)
    {
        if (slidingMenu->items)
        {
            int i;
            for (i = 0; i < slidingMenu->n_items; i++)
            {
                if (slidingMenu->items[i])
                    free(slidingMenu->items[i]);
            }
            free(slidingMenu->items);
        }

        if (slidingMenu->selects)
            free(slidingMenu->selects);

        if (slidingMenu->userdata && slidingMenu->freeUserData)
            slidingMenu->freeUserData(slidingMenu->userdata);
        free(slidingMenu);
    }
}

int SlidingMenu_Show(SlidingMenu *slidingMenu)
{
    if (!slidingMenu)
        return -1;

    // 如果已经打开了窗口，把它关闭
    if (slidingMenu->window)
    {
        GUI_SetWindowData(slidingMenu->window, NULL); // 设为NULL，防止onWindowClose时销毁slidingMenu
        GUI_CloseWindow(slidingMenu->window);
        slidingMenu->window = NULL;
    }

    slidingMenu->status = TYPE_SLIDING_MENU_STATUS_HIDE;

    // 创建窗口
    slidingMenu->window = GUI_CreateWindow();
    if (!slidingMenu->window)
        return -1;

    GUI_WindowCallbacks callbacks;
    memset(&callbacks, 0, sizeof(GUI_WindowCallbacks));
    callbacks.onOpen = onOpenWindow;
    callbacks.onClose = onCloseWindow;
    callbacks.onBeforeDraw = onBeforeDrawWindow;
    callbacks.onDraw = onDrawWindow;
    callbacks.onCtrl = onCtrlWindow;
    GUI_SetWindowCallbacks(slidingMenu->window, &callbacks);
    GUI_SetWindowData(slidingMenu->window, slidingMenu);

    if (GUI_OpenWindow(slidingMenu->window) < 0)
    {
        GUI_SetWindowData(slidingMenu->window, NULL);
        GUI_DestroyWindow(slidingMenu->window);
        slidingMenu->window = NULL;
        return -1;
    }

    return 0;
}

int SlidingMenu_Dismiss(SlidingMenu *slidingMenu)
{
    if (!slidingMenu)
        return -1;

    if (slidingMenu->status == TYPE_SLIDING_MENU_STATUS_DISMISS) // 已经在关闭中
        return 0;

    if (slidingMenu->status == TYPE_SLIDING_MENU_STATUS_SHOW)
        slidingMenu->status = TYPE_SLIDING_MENU_STATUS_DISMISS; // 设置状态，通过onEventWindow关闭
    else
        SlidingMenu_Destroy(slidingMenu); // 直接销毁

    return 0;
}

int SlidingMenu_Open(SlidingMenu *slidingMenu)
{
    if (!slidingMenu)
        return -1;

    slidingMenu->gradual_count = MAX_SLIDINGMENU_GRADUAL_COUNT; // 取消渐变动画
    return SlidingMenu_Show(slidingMenu);
}

int SlidingMenu_Close(SlidingMenu *slidingMenu)
{
    if (!slidingMenu)
        return -1;

    slidingMenu->gradual_count = 0; // 取消渐变动画
    return SlidingMenu_Dismiss(slidingMenu);
}

int SlidingMenu_SetAutoFree(SlidingMenu *slidingMenu, int auto_free)
{
    if (!slidingMenu)
        return -1;

    slidingMenu->dont_free = !auto_free;

    return 0;
}

int SlidingMenu_SetData(SlidingMenu *slidingMenu, void *data)
{
    if (!slidingMenu)
        return -1;

    slidingMenu->userdata = data;

    return 0;
}

int SlidingMenu_SetMode(SlidingMenu *slidingMenu, SlidingMenuModeType mode)
{
    if (!slidingMenu)
        return -1;

    slidingMenu->mode = mode;
    SlidingMenu_UpdateLayout(slidingMenu);

    return 0;
}

int SlidingMenu_SetChoiceType(SlidingMenu *slidingMenu, SlidingMenuChoiceType type)
{
    if (!slidingMenu)
        return -1;

    slidingMenu->choice_type = type;
    SlidingMenu_UpdateLayout(slidingMenu);

    return 0;
}

int SlidingMenu_SetItems(SlidingMenu *slidingMenu, char *const *items, int n_items)
{
    if (!slidingMenu)
        return -1;

    slidingMenu->focus_pos = 0;
    slidingMenu->listview_w = 0;
    slidingMenu->listview_h = 0;
    slidingMenu->listview_wrap_w = 0;
    slidingMenu->listview_wrap_h = 0;

    if (slidingMenu->items)
    {
        int i;
        for (i = 0; i < slidingMenu->n_items; i++)
        {
            if (slidingMenu->items[i])
                free(slidingMenu->items[i]);
        }
        free(slidingMenu->items);
        slidingMenu->items = NULL;
    }

    if (slidingMenu->selects)
    {
        free(slidingMenu->selects);
        slidingMenu->selects = NULL;
    }

    slidingMenu->n_items = 0;

    if (!items || n_items <= 0)
        return 0;

    slidingMenu->items = (char **)calloc(n_items, sizeof(char *));
    if (!slidingMenu->items)
        return -1;

    slidingMenu->selects = (int *)calloc(n_items, sizeof(int));
    if (!slidingMenu->selects)
    {
        free(slidingMenu->items);
        slidingMenu->items = NULL;
        return -1;
    }

    slidingMenu->n_items = n_items;

    int window_w = 0;
    GUI_GetWindowAvailableSize(&window_w, NULL);
    int itemview_h = ITEMS_ITEMVIEW_HEIGHT;
    int text_w = 0;

    int i;
    for (i = 0; i < n_items; i++)
    {
        if (items[i])
        {
            int w = GUI_GetTextWidth(items[i]);
            if (w > text_w)
                text_w = w;

            slidingMenu->items[i] = malloc(strlen(items[i]) + 1);
            if (slidingMenu->items[i])
                strcpy(slidingMenu->items[i], items[i]);
        }
    }

    int itemview_wrap_w = text_w + ITEMS_ITEMVIEW_PADDING_L * 2;
    int listview_wrap_w = itemview_wrap_w + ITEMS_LISTVIEW_PADDING_L * 2;
    int itemview_wrap_h = n_items * itemview_h;
    int listview_wrap_h = itemview_wrap_h + ITEMS_LISTVIEW_PADDING_T * 2;

    slidingMenu->listview_wrap_w = listview_wrap_w;
    slidingMenu->listview_wrap_h = listview_wrap_h;

    SlidingMenu_UpdateLayout(slidingMenu);

    return 0;
}

int SlidingMenu_SetOnItemClickListener(SlidingMenu *slidingMenu, int (*onItemClickListener)(SlidingMenu *slidingMenu, int which))
{
    if (!slidingMenu)
        return -1;

    slidingMenu->onItemClick = onItemClickListener;

    return 0;
}

int SlidingMenu_SetOnSelectChangedListener(SlidingMenu *slidingMenu, int (*onSelectChangedListener)(SlidingMenu *slidingMenu))
{
    if (!slidingMenu)
        return -1;

    slidingMenu->onSelectChanged = onSelectChangedListener;

    return 0;
}

int SlidingMenu_SetOnCloseListener(SlidingMenu *slidingMenu, int (*onCloseListener)(SlidingMenu *slidingMenu))
{
    if (!slidingMenu)
        return -1;

    slidingMenu->onClose = onCloseListener;

    return 0;
}

int SlidingMenu_SetFreeDataCallback(SlidingMenu *slidingMenu, void (*freeData)(void *data))
{
    if (!slidingMenu)
        return -1;

    slidingMenu->freeUserData = freeData;

    return 0;
}

int SlidingMenu_SetItemSeclected(SlidingMenu *slidingMenu, int id, int seclected)
{
    if (!slidingMenu || !slidingMenu->selects)
        return -1;

    if (id < 0 || id > slidingMenu->n_items - 1)
        return -1;

    slidingMenu->selects[id] = seclected;

    return 0;
}

int SlidingMenu_SetFocusPos(SlidingMenu *slidingMenu, int focus_pos)
{
    if (!slidingMenu)
        return -1;

    int itemviews_h = slidingMenu->listview_h - ITEMS_LISTVIEW_PADDING_T * 2;
    int itemviews_wrap_h = slidingMenu->listview_wrap_h - ITEMS_LISTVIEW_PADDING_T * 2;
    int itemview_h = ITEMS_ITEMVIEW_HEIGHT;

    if (focus_pos > slidingMenu->n_items - 1)
        focus_pos = slidingMenu->n_items - 1;
    if (focus_pos < 0)
        focus_pos = 0;

    if (itemviews_wrap_h < itemviews_h) // 列表长度小于视图，将列表居中
    {
        slidingMenu->listview_scroll_y = (itemviews_h - itemviews_wrap_h) / 2;
        slidingMenu->focus_pos = focus_pos;
        return 0;
    }

    // 相对的滚动偏移，不是真实的，需要起点加上它
    int scroll_y = 0 - itemview_h * focus_pos;      // 先顶部对齐
    scroll_y += (itemviews_h / 2 - itemview_h / 2); // 再设置居中

    // 修正scroll_y
    int max_srcoll_y = 0;                                      // 顶部不能越进
    int min_scroll_y = MIN(itemviews_h - itemviews_wrap_h, 0); // 底部不能越进，如果列表总显示长度大于可显示长度的话
    if (scroll_y < min_scroll_y)
        scroll_y = min_scroll_y;
    if (scroll_y > max_srcoll_y)
        scroll_y = max_srcoll_y;

    slidingMenu->listview_scroll_y = scroll_y;
    slidingMenu->focus_pos = focus_pos;

    return 0;
}

void *SlidingMenu_GetData(SlidingMenu *slidingMenu)
{
    return slidingMenu ? slidingMenu->userdata : NULL;
}

int SlidingMenu_GetItemsLength(SlidingMenu *slidingMenu)
{
    if (!slidingMenu)
        return -1;

    return slidingMenu->n_items;
}

int SlidingMenu_GetItems(SlidingMenu *slidingMenu, char *const **items, int *length)
{
    if (!slidingMenu || !items)
        return -1;

    *items = slidingMenu->items;
    *length = slidingMenu->n_items;

    return 0;
}

int SlidingMenu_IsItemSeclected(SlidingMenu *slidingMenu, int id)
{
    if (!slidingMenu || !slidingMenu->selects)
        return 0;

    if (id < 0 || id > slidingMenu->n_items - 1)
        return 0;

    return slidingMenu->selects[id];
}
