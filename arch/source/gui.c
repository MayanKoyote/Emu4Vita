#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <psp2/kernel/threadmgr.h>
#include <psp2/power.h>

#include "gui.h"
#include "browser.h"
#include "utils.h"
#include "config.h"
#include "app.h"

#define STATUS_BAR_PADDING_T 10
#define STATUS_BAR_PADDING_L 10
#define STATUS_BAR_HEIGHT (GUI_GetLineHeight() + STATUS_BAR_PADDING_T * 2)

#define MAIN_BG_COLOR 0x4F1F1F1F

#define MAIN_TITLE APP_NAME " " APP_VER

static SceKernelLwMutexWork gui_draw_mutex = {0};
static vita2d_texture *wallpaper_texture = NULL;

void GUI_DrawTopStatusBar(char *title)
{
    int view_sx = 0;
    int view_sy = 0;
    int view_dx = GUI_SCREEN_WIDTH;
    int view_dy = STATUS_BAR_HEIGHT;

    vita2d_draw_line(view_sx, view_dy, view_dx, view_dy, WHITE);

    int sx = view_sx + STATUS_BAR_PADDING_L;
    int sy = view_sy + STATUS_BAR_PADDING_T;
    GUI_DrawText(sx, sy, WHITE, title);

    sx = view_dx - STATUS_BAR_PADDING_L;
    if (!IsVitatvModel())
    {
        uint32_t color;
        if (scePowerIsBatteryCharging())
            color = YELLOW;
        else if (scePowerIsLowBattery())
            color = RED;
        else
            color = GREEN;

        int percent = scePowerGetBatteryLifePercent();
        char battery_string[24];
        snprintf(battery_string, sizeof(battery_string), "%d%%", percent);
        sx -= GUI_GetTextWidth(battery_string);
        GUI_DrawText(sx, sy, color, battery_string);
        sx -= STATUS_BAR_PADDING_L;
    }

    // Date & time
    SceDateTime time;
    sceRtcGetCurrentClock(&time, 0);

    char date_string[24];
    GetDateString(date_string, date_format, &time);

    char time_string[16];
    GetTimeString(time_string, time_format, &time);

    char string[64];
    snprintf(string, sizeof(string), "%s  %s", date_string, time_string);
    sx -= GUI_GetTextWidth(string);
    GUI_DrawText(sx, sy, WHITE, string);
}

void GUI_DrawBottomStatusBar()
{
    int view_sx = 0;
    int view_sy = GUI_SCREEN_HEIGHT - STATUS_BAR_HEIGHT;
    int view_dx = GUI_SCREEN_WIDTH;

    vita2d_draw_line(view_sx, view_sy, view_dx, view_sy, WHITE);

    int sx = view_sx + STATUS_BAR_PADDING_L;
    int sy = view_sy + STATUS_BAR_PADDING_T;
    GUI_DrawTextf(sx, sy, WHITE, "Built on  %s  %s", BUILD_DATE, REPOSITORY_ADDRESS);
}

void GUI_DrawMain()
{
    GUI_LockDrawMutex();
    GUI_StartDrawing();

    if (wallpaper_texture)
    {
        float x_scale = (float)GUI_SCREEN_WIDTH / (float)vita2d_texture_get_width(wallpaper_texture);
        float y_scale = (float)GUI_SCREEN_HEIGHT / (float)vita2d_texture_get_height(wallpaper_texture);
        vita2d_draw_texture_scale(wallpaper_texture, 0, 0, x_scale, y_scale);
    }
    vita2d_draw_rectangle(0, 0, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT, MAIN_BG_COLOR);

    BrowserDraw();

    GUI_DrawTopStatusBar(MAIN_TITLE);
    GUI_DrawBottomStatusBar();

    GUI_EndDrawing();
    GUI_RenderPresent();
    GUI_UnlockDrawMutex();
}

void GUI_CtrlMain()
{
    ReadPad();
    BrowserCtrl();
}

void GUI_RunMain()
{
    GUI_DrawMain();
    GUI_CtrlMain();
}

void GUI_SetWallpaperTexture(vita2d_texture *texture)
{
    wallpaper_texture = texture;
}

int GUI_InitDraw()
{
    sceKernelCreateLwMutex(&gui_draw_mutex, "gui_draw_mutex", 2, 0, NULL);
    return 0;
}

int GUI_DeinitDraw()
{
    sceKernelDeleteLwMutex(&gui_draw_mutex);
    return 0;
}

int GUI_Init()
{
    GUI_InitLib();
    GUI_InitDraw();
    BrowserInit();

    return 0;
}

void GUI_Deinit()
{
    BrowserDeinit();
    GUI_DeinitDraw();
    GUI_DeinitLib();

    if (wallpaper_texture != NULL)
    {
        vita2d_free_texture(wallpaper_texture);
        wallpaper_texture = NULL;
    }
}

int GUI_GetActivityLayoutPosition(int *x, int *y)
{
    if (x)
        *x = 0;
    if (y)
        *y = STATUS_BAR_HEIGHT;

    return 0;
}

int GUI_GetActivityAvailableSize(int *w, int *h)
{
    if (w)
        *w = GUI_SCREEN_WIDTH;
    if (h)
        *h = GUI_SCREEN_HEIGHT - STATUS_BAR_HEIGHT * 2;

    return 0;
}

int GUI_LockDrawMutex()
{
    return sceKernelLockLwMutex(&gui_draw_mutex, 1, NULL);
}

int GUI_UnlockDrawMutex()
{
    return sceKernelUnlockLwMutex(&gui_draw_mutex, 1);
}
