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
#include "init.h"
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
    SettingMenu *visible_menus; // 可见的menus
    int n_visible_menus;
    SettingMenuItem *visible_items; // 可见的items
    int n_visible_items;
    int listview_wrap_h;
    int listview_scroll_y;
};

// Window
#define WINDOW_WIDTH 960.0f  // 840.0f
#define WINDOW_HEIGHT 544.0f // 476.0f
#define WINDOW_PADDING 0.0f
#define WINDOW_SX (GUI_SCREEN_WIDTH - WINDOW_WIDTH) / 2
#define WINDOW_SY (GUI_SCREEN_HEIGHT - WINDOW_HEIGHT) / 2
#define WINDOW_DX (WINDOW_SX + WINDOW_WIDTH)
#define WINDOW_DY (WINDOW_SY + WINDOW_HEIGHT)
#define WINDOW_BG_COLOR COLOR_ALPHA(COLOR_BLACK, 0xAF)

#define MENU_LAYOUT_CHILD_MARGIN 8.0f

#define MENU_TABVIEW_PADDING_L 10.0f
#define MENU_TABVIEW_PADDING_T 10.0f
#define MENU_TABVIEW_DIVIER_SIZE 3.0f
#define MENU_TABVIEW_HEIGHT (GUI_GetLineHeight() + MENU_TABVIEW_PADDING_T * 2 + MENU_TABVIEW_DIVIER_SIZE)

#define MENU_LISTVIEW_PADDING_L 0.0f
#define MENU_LISTVIEW_PADDING_T 8.0f

#define MENU_ITEMVIEW_PADDING_L 10.0f
#define MENU_ITEMVIEW_PADDING_T 6.0f
#define MENU_ITEMVIEW_HEIGHT (GUI_GetLineHeight() + MENU_ITEMVIEW_PADDING_T * 2)

#define MENU_ITEMVIEW_COLOR_FOCUS_BG GUI_DEF_COLOR_FOCUS

extern int Setting_UpdateMenu(SettingMenu *menu);

static int Setting_UpdateWindowLayout(SettingWindow *window)
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

    window->listview_wrap_h = itemviews_h + MENU_LISTVIEW_PADDING_T * 2;
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
        int n = 0;
        do
        {
            if (pos > 0)
                pos--;
            else
                break;
            n++;
        } while (!SETTING_IS_VISIBLE(items[pos].visibility) && n < menu->n_items); // 防止死循环
    }
    else if (move_type == TYPE_MOVE_DOWN)
    {
        int n = 0;
        do
        {
            if (pos < menu->n_items - 1)
                pos++;
            else
                break;
            n++;
        } while (!SETTING_IS_VISIBLE(items[pos].visibility) && n < menu->n_items); // 防止死循环
    }
    else // 尝试修正pos
    {
        if (pos > n_items - 1)
            pos = n_items - 1;
        if (pos < 0)
            pos = 0;
        while (!SETTING_IS_VISIBLE(items[pos].visibility) && pos < n_items - 1)
            pos++;
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
        } while (!SETTING_IS_VISIBLE(menus[pos].visibility) && n < n_menus);
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
        } while (!SETTING_IS_VISIBLE(menus[pos].visibility) && n < n_menus);
    }
    else // 尝试修正pos
    {
        if (pos > n_menus - 1)
            pos = n_menus - 1;
        if (pos < 0)
            pos = 0;
        while (!SETTING_IS_VISIBLE(menus[pos].visibility) && pos < n_menus - 1)
            pos++;
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

    st_window->status = TYPE_SETTING_WINDOW_STATUS_SHOW;

    SettingContext *context = st_window->context;
    if (!context || !context->menus)
        return -1;

    SettingMenu *menus = context->menus;
    int n_menus = context->n_menus;

    int i;
    for (i = 0; i < n_menus; i++)
    {
        if (SETTING_IS_VISIBLE(menus[i].visibility) && menus[i].onStart)
            menus[i].onStart(&menus[i]);
    }

    moveTabBarPos(context, TYPE_MOVE_NONE);
    Setting_UpdateWindowLayout(st_window);

    return 0;
}

static int onCloseWindow(GUI_Window *window)
{
    SettingWindow *st_window = (SettingWindow *)GUI_GetWindowData(window);
    if (!st_window)
        return -1;

    SettingContext *context = st_window->context;
    if (!context || !context->menus)
        return -1;

    SettingMenu *menus = context->menus;
    int n_menus = context->n_menus;

    if (context->menus)
    {
        int i;
        for (i = 0; i < n_menus; i++)
        {
            if (SETTING_IS_VISIBLE(menus[i].visibility) && menus[i].onFinish)
                menus[i].onFinish(&menus[i]);
        }
    }

    if (Emu_IsGameLoaded())
        Emu_ResumeGame();

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
    int divier_size = MENU_TABVIEW_DIVIER_SIZE;
    int layout_w = w;
    int layout_h = h - divier_size;
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
    GetTimeString(time_string, time_format, &time);
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

    // Draw divier
    GUI_DrawFillRectangle(layout_sx, layout_dy, layout_w, divier_size, MENU_ITEMVIEW_COLOR_FOCUS_BG);

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
                GUI_DrawFillRectangle(itemview_x, itemview_y, itemviews_w, itemview_h, MENU_ITEMVIEW_COLOR_FOCUS_BG);

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
    if (context->menus_pos == ID_SETTING_MENU_STATE)
        Setting_DrawState();
    else
        drawMenu(st_window, x, y, w, h);

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

    SettingMenu *menu = &context->menus[context->menus_pos];

    if (released_pad[PAD_PSBUTTON])
    {
        if (GUI_IsPsbuttonEnabled())
        {
            if (context->menus_pos != 0)
            {
                context->menus_pos = 0;
                moveTabBarPos(context, TYPE_MOVE_NONE);
                Setting_UpdateWindowLayout(st_window);
            }
            else
            {
                GUI_CloseWindow(window);
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
        Setting_UpdateWindowLayout(st_window);
    }
    else if (pressed_pad[PAD_R1])
    {
        moveTabBarPos(context, TYPE_MOVE_DOWN);
        Setting_UpdateWindowLayout(st_window);
    }

    if (menu == &context->menus[ID_SETTING_MENU_STATE])
    {
        Setting_CtrlState();
    }
    else
    {
        if (hold_pad[PAD_UP] || hold2_pad[PAD_LEFT_ANALOG_UP])
        {
            moveMenuPos(menu, TYPE_MOVE_UP);
            Setting_UpdateWindowLayout(st_window);
        }
        else if (hold_pad[PAD_DOWN] || hold2_pad[PAD_LEFT_ANALOG_DOWN])
        {
            moveMenuPos(menu, TYPE_MOVE_DOWN);
            Setting_UpdateWindowLayout(st_window);
        }
        else if (released_pad[PAD_ENTER])
        {
            SettingMenuItem *menu_item = &menu->items[menu->menu_pos];

            if (menu_item->onItemClick)
                menu_item->onItemClick(menu, menu_item, menu->menu_pos);
        }
        else if (released_pad[PAD_CANCEL])
        {
            Setting_CloseWindow(st_window);
        }
    }

    return 0;
}

static int onEventWindow(GUI_Window *window)
{
    SettingWindow *st_window = (SettingWindow *)GUI_GetWindowData(window);
    if (!st_window)
        return -1;

    if (st_window->status == TYPE_SETTING_WINDOW_STATUS_DISMISS)
    {
        GUI_CloseWindow(window);
        return 0;
    }

    SettingContext *context = st_window->context;
    if (!context)
        return -1;

    return 0;
}

SettingWindow *Setting_CreateWindow()
{
    SettingWindow *st_window = (SettingWindow *)calloc(1, sizeof(SettingWindow));
    if (!st_window)
        return NULL;

    return st_window;
}

int Setting_DestroyWindow(SettingWindow *window)
{
    if (!window)
        return -1;

    if (window->window)
    {
        GUI_SetWindowData(window->window, NULL); // 设置为NULL，防止onWindowClose时销毁dialog
        GUI_CloseWindow(window->window);         // 关闭窗口
        window->window = NULL;
    }

    if (!window->dont_free)
    {
        free(window);
    }

    return 0;
}

int Setting_OpenWindow(SettingWindow *window)
{
    if (!window)
        return -1;

    if (window->window)
    {
        GUI_SetWindowData(window->window, NULL);
        GUI_CloseWindow(window->window);
        window->window = NULL;
    }

    window->status = TYPE_SETTING_WINDOW_STATUS_HIDE;

    window->window = GUI_CreateWindow();
    if (!window->window)
        return -1;

    GUI_WindowCallbacks callbacks;
    memset(&callbacks, 0, sizeof(GUI_WindowCallbacks));
    callbacks.onOpen = onOpenWindow;
    callbacks.onClose = onCloseWindow;
    callbacks.onDraw = onDrawWindow;
    callbacks.onCtrl = onCtrlWindow;
    callbacks.onEvent = onEventWindow;
    GUI_SetWindowCallbacks(window->window, &callbacks);
    GUI_SetWindowData(window->window, window);

    if (GUI_OpenWindow(window->window) < 0)
    {
        GUI_SetWindowData(window->window, NULL);
        GUI_DestroyWindow(window->window);
        window->window = NULL;
        return -1;
    }

    return 0;
}

int Setting_CloseWindow(SettingWindow *window)
{
    if (!window)
        return -1;

    if (window->status == TYPE_SETTING_WINDOW_STATUS_DISMISS) // 已经在关闭中
        return -1;

    if (window->status == TYPE_SETTING_WINDOW_STATUS_SHOW)
        window->status = TYPE_SETTING_WINDOW_STATUS_DISMISS; // 设置状态，通过onEventWindow关闭
    else
        Setting_DestroyWindow(window); // 直接销毁

    return 0;
}

int Setting_SetWindowAutoFree(SettingWindow *window, int auto_free)
{
    if (!window)
        return -1;

    window->dont_free = !auto_free;

    return 0;
}

int Setting_SetWindowContext(SettingWindow *window, SettingContext *context)
{
    if (!window)
        return -1;

    window->context = context;

    return 0;
}

int Setting_GetWindowMenuLayoutPosition(int *layout_x, int *layout_y)
{
    if (layout_x)
        *layout_x = WINDOW_SX;
    if (layout_y)
        *layout_y = WINDOW_SY + MENU_TABVIEW_HEIGHT;
    return 0;
}

int Setting_GetWindowMenuAvailableSize(int *available_w, int *available_h)
{
    if (available_w)
        *available_w = WINDOW_WIDTH;
    if (available_h)
        *available_h = WINDOW_HEIGHT - MENU_TABVIEW_HEIGHT;
    return 0;
}
