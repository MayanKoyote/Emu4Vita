#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <psp2/power.h>
#include <psp2/rtc.h>

#include "gui/gui.h"
#include "emu/emu.h"
#include "setting_types.h"
#include "setting_window.h"
#include "setting_callbacks.h"
#include "setting_state.h"
#include "utils.h"
#include "app.h"
#include "boot.h"

typedef enum SettingWindowStatusType
{
    TYPE_SETTING_WINDOW_STATUS_HIDE,
    TYPE_SETTING_WINDOW_STATUS_SHOW,
    TYPE_SETTING_WINDOW_STATUS_DISMISS,
} SettingWindowStatusType;

struct SettingWindow
{
    int dont_free;
    GUI_Window *window;
    SettingContext *context;
    int status;
    int listview_wrap_h;
    int listview_scroll_y;
};

// Window
#define WINDOW_WIDTH 960  // 840
#define WINDOW_HEIGHT 544 // 476
#define WINDOW_PADDING 0
#define WINDOW_SX (GUI_SCREEN_WIDTH - WINDOW_WIDTH) / 2
#define WINDOW_SY (GUI_SCREEN_HEIGHT - WINDOW_HEIGHT) / 2
#define WINDOW_DX (WINDOW_SX + WINDOW_WIDTH)
#define WINDOW_DY (WINDOW_SY + WINDOW_HEIGHT)
#define WINDOW_BG_COLOR COLOR_ALPHA(COLOR_BLACK, 0xAF)

#define MENU_LAYOUT_CHILD_MARGIN 8

#define MENU_TABVIEW_PADDING_L 10
#define MENU_TABVIEW_PADDING_T 10
#define MENU_TABVIEW_DIVIDER_SIZE 3
#define MENU_TABVIEW_HEIGHT (GUI_GetLineHeight() + MENU_TABVIEW_PADDING_T * 2 + MENU_TABVIEW_DIVIDER_SIZE)

#define MENU_LISTVIEW_PADDING_L 0
#define MENU_LISTVIEW_PADDING_T 8

#define MENU_ITEMVIEW_PADDING_L 10
#define MENU_ITEMVIEW_PADDING_T 6
#define MENU_ITEMVIEW_HEIGHT (GUI_GetLineHeight() + MENU_ITEMVIEW_PADDING_T * 2)

#define MENU_ITEMVIEW_COLOR_FOCUS_BG GUI_DEF_COLOR_FOCUS

extern int Setting_UpdateMenu(SettingMenu *menu);
extern int Setting_StartContext(SettingContext *context);
extern int Setting_FinishContext(SettingContext *context);
extern int Setting_IsResumeGameEnabled();

int SettingWindow_UpdateLayout(SettingWindow *window)
{
    if (!window)
        return -1;

    window->listview_wrap_h = 0;
    window->listview_scroll_y = 0;

    SettingContext *context = window->context;
    if (!context || !context->menus)
        return -1;

    SettingMenu *menu = &context->menus[context->menus_pos];
    if (!menu)
        return -1;

    SettingMenuItem *items = menu->items;
    int n_items = menu->n_items;
    if (!items || n_items <= 0)
        return 0;

    int itemviews_h = WINDOW_HEIGHT - MENU_TABVIEW_HEIGHT - MENU_LISTVIEW_PADDING_T * 2;
    int itemview_h = MENU_ITEMVIEW_HEIGHT;
    int itemviews_wrap_h = 0;
    int scroll_y = 0;

    int i;
    for (i = 0; i < n_items; i++)
    {
        if (i == menu->menu_pos)
            scroll_y = 0 - itemviews_wrap_h;
        if (SETTING_IS_VISIBLE(items[i].visibility))
            itemviews_wrap_h += itemview_h;
    }

    scroll_y += (itemviews_h / 2) - (itemview_h / 2);

    // 修正scroll_y
    int max_srcoll_y = 0;                                      // 顶部不能越进
    int min_scroll_y = MIN(itemviews_h - itemviews_wrap_h, 0); // 底部不能越进，如果列表总显示长度大于可显示长度的话
    if (scroll_y < min_scroll_y)
        scroll_y = min_scroll_y;
    if (scroll_y > max_srcoll_y)
        scroll_y = max_srcoll_y;

    window->listview_wrap_h = itemviews_wrap_h + MENU_LISTVIEW_PADDING_T * 2;
    window->listview_scroll_y = scroll_y;

    return 0;
}

static int moveMenuPos(SettingMenu *menu, int move_type)
{
    if (!menu)
        return -1;

    if (!menu->items || menu->n_items <= 0)
    {
        menu->menu_pos = 0;
        return -1;
    }

    SettingMenuItem *items = menu->items;
    int n_items = menu->n_items;
    int pos = menu->menu_pos;

    if (move_type == TYPE_MOVE_UP)
    {
        do
        {
            if (pos > 0)
                pos--;
            else
                break;
        } while (!SETTING_IS_VISIBLE(items[pos].visibility));
    }
    else if (move_type == TYPE_MOVE_DOWN)
    {
        do
        {
            if (pos < n_items - 1)
                pos++;
            else
                break;
        } while (!SETTING_IS_VISIBLE(items[pos].visibility));
    }
    else // 尝试修正pos
    {
        if (pos > n_items - 1)
            pos = n_items - 1;
        if (pos < 0)
            pos = 0;
        int n = 0;
        while (!SETTING_IS_VISIBLE(items[pos].visibility) && n < n_items) // n < n_items 防止死循环
        {
            if (pos < n_items - 1)
                pos++;
            else
                pos = 0;
            n++;
        }
    }

    if (SETTING_IS_VISIBLE(items[pos].visibility))
        menu->menu_pos = pos;

    return 0;
}

static int moveTabBarPos(SettingContext *context, int move_type)
{
    if (!context)
        return -1;

    if (!context->menus || context->n_menus <= 0)
    {
        context->menus_pos = 0;
        return 0;
    }

    SettingMenu *menus = context->menus;
    int n_menus = context->n_menus;
    int pos = context->menus_pos;

    if (move_type == TYPE_MOVE_UP)
    {
        int n = 0;
        do
        {
            if (pos > 0)
                pos--;
            else
                pos = n_menus - 1;
            n++;
        } while (!SETTING_IS_VISIBLE(menus[pos].visibility) && n < n_menus); // n < n_menus 防止死循环
    }
    else if (move_type == TYPE_MOVE_DOWN)
    {
        int n = 0;
        do
        {
            if (pos < n_menus - 1)
                pos++;
            else
                pos = 0;
            n++;
        } while (!SETTING_IS_VISIBLE(menus[pos].visibility) && n < n_menus); // n < n_menus 防止死循环
    }
    else // 尝试修正pos
    {
        if (pos > n_menus - 1)
            pos = n_menus - 1;
        if (pos < 0)
            pos = 0;
        int n = 0;
        while (!SETTING_IS_VISIBLE(menus[pos].visibility) && n < n_menus - 1) // n < n_menus - 1 防止死循环
        {
            if (pos < n_menus - 1)
                pos++;
            else
                pos = 0;
            n++;
        }
    }

    if (SETTING_IS_VISIBLE(menus[pos].visibility))
    {
        context->menus_pos = pos;
        Setting_UpdateMenu(&context->menus[context->menus_pos]);
        moveMenuPos(&context->menus[context->menus_pos], TYPE_MOVE_NONE);
    }

    return 0;
}

static int onOpenWindow(GUI_Window *window)
{
    SettingWindow *st_window = (SettingWindow *)GUI_GetWindowData(window);
    if (!st_window)
        return -1;

    Setting_StartContext(st_window->context);
    moveTabBarPos(st_window->context, TYPE_MOVE_NONE);
    SettingWindow_UpdateLayout(st_window);

    st_window->status = TYPE_SETTING_WINDOW_STATUS_SHOW;

    return 0;
}

static int onCloseWindow(GUI_Window *window)
{
    SettingWindow *st_window = (SettingWindow *)GUI_GetWindowData(window);
    if (!st_window)
        return -1;

    Setting_FinishContext(st_window->context);

    st_window->window = NULL;
    GUI_SetWindowData(window, NULL);
    SettingWindow_Destroy(st_window);

    if (Emu_IsGameLoaded() && Setting_IsResumeGameEnabled())
        Emu_ResumeGame();

    return 0;
}

static int onBeforeDrawWindow(GUI_Window *window)
{
    SettingWindow *st_window = (SettingWindow *)GUI_GetWindowData(window);
    if (!st_window)
        return -1;

    if (st_window->status == TYPE_SETTING_WINDOW_STATUS_DISMISS)
    {
        GUI_CloseWindow(window);
        return 0;
    }

    return 0;
}

static int drawTabBar(SettingWindow *st_window, int x, int y, int w, int h)
{
    SettingContext *context = st_window->context;
    if (!context || !context->menus)
        return -1;

    GUI_SetClipping(x, y, w, h); // Set tabview clip

    SettingMenu *menus = context->menus;
    int n_menus = context->n_menus;
    int menus_pos = context->menus_pos;
    int divider_size = MENU_TABVIEW_DIVIDER_SIZE;
    int layout_w = w;
    int layout_h = h - divider_size;
    int layout_sx = x;
    int layout_sy = y;
    int layout_dx = layout_sx + layout_w;
    int layout_dy = layout_sy + layout_h;

    GUI_SetClipping(layout_sx, layout_sy, layout_w, layout_h); // Set layout clip

    int available_w = layout_w;
    int view_x = layout_dx;
    int view_y = layout_sy;
    int view_w;
    int view_h = layout_h;
    int text_x, text_y;

    text_y = view_y + MENU_TABVIEW_PADDING_T;

    // Draw battery
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
        char battery_string[24];
        snprintf(battery_string, sizeof(battery_string), "%d%%", percent);
        view_w = GUI_GetTextWidth(battery_string) + MENU_TABVIEW_PADDING_L;
        view_x -= view_w;
        text_x = view_x;
        GUI_DrawText(text_x, text_y, color, battery_string);
        available_w -= view_w;
    }

    // Draw time
    SceDateTime time;
    sceRtcGetCurrentClock(&time, 0);
    char time_string[16];
    GetTimeString(time_string, system_time_format, &time);
    view_w = GUI_GetTextWidth(time_string) + MENU_TABVIEW_PADDING_L;
    available_w -= view_w;
    view_x -= view_w;
    text_x = view_x;
    GUI_DrawText(text_x, text_y, GUI_DEF_COLOR_TEXT, time_string);

    available_w -= MENU_TABVIEW_PADDING_L;
    view_x = layout_sx;

    // Draw tab
    if (menus)
    {
        GUI_SetClipping(view_x, view_y, available_w, view_h);

        int i;
        for (i = 0; i < n_menus; i++)
        {
            if (!SETTING_IS_VISIBLE(menus[i].visibility))
                continue;

            char *name = GetLangString(&menus[i].name);
            int ww = MENU_TABVIEW_PADDING_L * 2;
            if (name)
                ww += GUI_GetTextWidth(name);
            if (i == menus_pos)
                GUI_DrawFillRectangle(view_x, view_y, ww, view_h, MENU_ITEMVIEW_COLOR_FOCUS_BG);
            if (name)
            {
                text_x = view_x + MENU_TABVIEW_PADDING_L;
                GUI_DrawText(text_x, text_y, COLOR_WHITE, name);
            }
            view_x += ww;
        }

        GUI_UnsetClipping();
    }

    GUI_UnsetClipping(); // Unset layout clip

    // Draw divider
    GUI_DrawFillRectangle(layout_sx, layout_dy, layout_w, divider_size, MENU_ITEMVIEW_COLOR_FOCUS_BG);

    GUI_UnsetClipping(); // Unset tabview clip

    return 0;
}

static int drawMenu(SettingWindow *st_window, int x, int y, int w, int h)
{
    SettingContext *context = st_window->context;
    if (!context || !context->menus)
        return -1;

    SettingMenu *menu = &context->menus[context->menus_pos];
    if (!menu || !menu->items)
        return 0;

    SettingMenuItem *items = menu->items;
    int n_items = menu->n_items;
    int menu_pos = menu->menu_pos;
    int layout_w = w;
    int layout_h = h;
    int layout_sx = x;
    int layout_sy = y;
    int layout_dx = layout_sx + layout_w;
    int layout_dy = layout_sy + layout_h;

    GUI_SetClipping(layout_sx, layout_sy, layout_w, layout_h); // Set layout clip

    int itemviews_sx = layout_sx + MENU_LISTVIEW_PADDING_L;
    int itemviews_sy = layout_sy + MENU_LISTVIEW_PADDING_T;
    int itemviews_dx = layout_dx - MENU_LISTVIEW_PADDING_L;
    int itemviews_dy = layout_dy - MENU_LISTVIEW_PADDING_T;
    int itemviews_w = itemviews_dx - itemviews_sx;
    int itemviews_h = itemviews_dy - itemviews_sy;
    int itemviews_wrap_h = st_window->listview_wrap_h - MENU_LISTVIEW_PADDING_T * 2;
    int itemview_w = itemviews_w;
    int itemview_h = MENU_ITEMVIEW_HEIGHT;

    int itemview_x = itemviews_sx;
    int itemview_y = itemviews_sy + st_window->listview_scroll_y;
    int itemview_dx = itemviews_dx;
    int text_max_w = (itemviews_w - MENU_ITEMVIEW_PADDING_L * 3) / 2;
    int text_x = itemview_x + MENU_ITEMVIEW_PADDING_L;
    int text_dx = itemview_dx - MENU_ITEMVIEW_PADDING_L;
    int text_x2 = text_dx - text_max_w;
    int text_y;
    int text_w;
    int text_h = itemview_h - MENU_ITEMVIEW_PADDING_T;

    // Draw tab
    if (items)
    {
        GUI_SetClipping(itemviews_sx, itemviews_sy, itemviews_w, itemviews_h); // Set itemviews clip

        int i;
        for (i = 0; i < n_items; i++)
        {
            if (!SETTING_IS_VISIBLE(items[i].visibility))
                continue;
            if (itemview_y + itemview_h < itemviews_sy)
                goto NEXT;
            if (itemview_y > itemviews_dy)
                break;

            if (i == menu_pos)
                GUI_DrawFillRectangle(itemview_x, itemview_y, itemview_w, itemview_h, MENU_ITEMVIEW_COLOR_FOCUS_BG);

            text_y = itemview_y + MENU_ITEMVIEW_PADDING_T;

            char *name = GetLangString(&items[i].name);
            if (name)
            {
                GUI_SetClipping(text_x, text_y, text_max_w, text_h);
                GUI_DrawText(text_x, text_y, COLOR_WHITE, name);
                GUI_UnsetClipping();
            }
            char *name2 = GetLangString(&items[i].option_name);
            if (name2)
            {
                GUI_SetClipping(text_x2, text_y, text_max_w, text_h);
                text_w = GUI_GetTextWidth(name2);
                GUI_DrawText(MAX(text_dx - text_w, text_x2), text_y, COLOR_WHITE, name2);
                GUI_UnsetClipping();
            }
        NEXT:
            itemview_y += itemview_h;
        }

        GUI_UnsetClipping(); // Unset itemviews clip

        // Draw scrollbar
        int track_x = layout_dx - GUI_DEF_SCROLLBAR_SIZE;
        int track_y = layout_sy;
        int track_h = layout_h;
        GUI_DrawVerticalScrollbar(track_x, track_y, track_h, itemviews_wrap_h, itemviews_h, 0 - st_window->listview_scroll_y, 0);
    }

    GUI_UnsetClipping(); // Unset layout clip

    return 0;
}

static int onDrawWindow(GUI_Window *window)
{
    SettingWindow *st_window = (SettingWindow *)GUI_GetWindowData(window);
    if (!st_window)
        return -1;

    GUI_DrawFillRectangle(WINDOW_SX, WINDOW_SY, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_BG_COLOR);

    SettingContext *context = st_window->context;
    if (!context)
        return -1;

    int x = WINDOW_SX;
    int y = WINDOW_SY;
    int w = WINDOW_WIDTH;
    int h = MENU_TABVIEW_HEIGHT;
    drawTabBar(st_window, x, y, w, h);

    y += h;
    h = WINDOW_DY - y;
    SettingMenu *menu = &context->menus[context->menus_pos];
    if (menu)
    {
        if (menu->onDraw)
            menu->onDraw(menu);
        else
            drawMenu(st_window, x, y, w, h);
    }

    return 0;
}

static int onCtrlWindow(GUI_Window *window)
{
    SettingWindow *st_window = (SettingWindow *)GUI_GetWindowData(window);
    if (!st_window)
        return -1;

    SettingContext *context = st_window->context;
    if (!context || !context->menus)
        return -1;

    if (released_pad[PAD_HOME])
    {
        if (GUI_IsHomeKeyEnabled())
        {
            if (context->menus_pos != 0)
            {
                context->menus_pos = 0;
                moveTabBarPos(context, TYPE_MOVE_NONE);
                SettingWindow_UpdateLayout(st_window);
            }
            else
            {
                SettingWindow_Close(st_window);
            }
        }
    }
    else if (released_pad[PAD_SELECT])
    {
        if (BootGetMode() == BOOT_MODE_ARCH && !Emu_IsGameLoaded())
            Setting_onExitToArchItemClick(NULL, NULL, 0);
        else
            Setting_onExitGameItemClick(NULL, NULL, 0);
    }
    else if (released_pad[PAD_START])
    {
        Setting_onExitAppItemClick(NULL, NULL, 0);
    }
    else if (pressed_pad[PAD_L1])
    {
        moveTabBarPos(context, TYPE_MOVE_UP);
        SettingWindow_UpdateLayout(st_window);
    }
    else if (pressed_pad[PAD_R1])
    {
        moveTabBarPos(context, TYPE_MOVE_DOWN);
        SettingWindow_UpdateLayout(st_window);
    }
    else
    {
        SettingMenu *menu = &context->menus[context->menus_pos];

        if (menu && menu->onCtrl)
        {
            menu->onCtrl(menu);
        }
        else if (released_pad[PAD_CANCEL])
        {
            SettingWindow_Close(st_window);
        }
        else if (menu && menu->items)
        {
            if (hold_pad[PAD_UP] || hold2_pad[PAD_LEFT_ANALOG_UP])
            {
                moveMenuPos(menu, TYPE_MOVE_UP);
                SettingWindow_UpdateLayout(st_window);
            }
            else if (hold_pad[PAD_DOWN] || hold2_pad[PAD_LEFT_ANALOG_DOWN])
            {
                moveMenuPos(menu, TYPE_MOVE_DOWN);
                SettingWindow_UpdateLayout(st_window);
            }
            else if (released_pad[PAD_ENTER])
            {
                SettingMenuItem *menu_item = &menu->items[menu->menu_pos];
                if (menu_item && menu_item->onItemClick)
                    menu_item->onItemClick(menu, menu_item, menu->menu_pos);
            }
        }
    }

    return 0;
}

SettingWindow *SettingWindow_Create()
{
    SettingWindow *st_window = (SettingWindow *)calloc(1, sizeof(SettingWindow));
    return st_window;
}

void SettingWindow_Destroy(SettingWindow *st_window)
{
    if (!st_window)
        return;

    if (st_window->window)
    {
        GUI_CloseWindow(st_window->window);
        return;
    }

    if (!st_window->dont_free)
    {
        free(st_window);
    }
}

int SettingWindow_Open(SettingWindow *st_window)
{
    if (!st_window)
        return -1;

    if (st_window->window)
    {
        GUI_SetWindowData(st_window->window, NULL);
        GUI_CloseWindow(st_window->window);
        st_window->window = NULL;
    }

    st_window->status = TYPE_SETTING_WINDOW_STATUS_HIDE;

    st_window->window = GUI_CreateWindow();
    if (!st_window->window)
        return -1;

    GUI_WindowCallbacks callbacks;
    memset(&callbacks, 0, sizeof(GUI_WindowCallbacks));
    callbacks.onOpen = onOpenWindow;
    callbacks.onClose = onCloseWindow;
    callbacks.onBeforeDraw = onBeforeDrawWindow;
    callbacks.onDraw = onDrawWindow;
    callbacks.onCtrl = onCtrlWindow;
    GUI_SetWindowCallbacks(st_window->window, &callbacks);
    GUI_SetWindowData(st_window->window, st_window);

    if (GUI_OpenWindow(st_window->window) < 0)
    {
        GUI_SetWindowData(st_window->window, NULL);
        GUI_DestroyWindow(st_window->window);
        st_window->window = NULL;
        return -1;
    }

    return 0;
}

int SettingWindow_Close(SettingWindow *st_window)
{
    if (!st_window)
        return -1;

    if (st_window->status == TYPE_SETTING_WINDOW_STATUS_DISMISS) // 已经在关闭中
        return 0;

    if (st_window->status == TYPE_SETTING_WINDOW_STATUS_SHOW)
        st_window->status = TYPE_SETTING_WINDOW_STATUS_DISMISS; // 设置状态为关闭
    else
        SettingWindow_Destroy(st_window); // 直接销毁

    return 0;
}

int SettingWindow_SetAutoFree(SettingWindow *st_window, int auto_free)
{
    if (!st_window)
        return -1;

    st_window->dont_free = !auto_free;

    return 0;
}

int SettingWindow_SetContext(SettingWindow *st_window, SettingContext *context)
{
    if (!st_window)
        return -1;

    st_window->context = context;

    return 0;
}

int SettingWindow_GetMenuLayoutPosition(int *layout_x, int *layout_y)
{
    if (layout_x)
        *layout_x = WINDOW_SX;
    if (layout_y)
        *layout_y = WINDOW_SY + MENU_TABVIEW_HEIGHT;
    return 0;
}

int SettingWindow_GetMenuAvailableSize(int *available_w, int *available_h)
{
    if (available_w)
        *available_w = WINDOW_WIDTH;
    if (available_h)
        *available_h = WINDOW_HEIGHT - MENU_TABVIEW_HEIGHT;
    return 0;
}
