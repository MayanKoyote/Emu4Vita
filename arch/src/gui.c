#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <psp2/ctrl.h>
#include <psp2/power.h>
#include <psp2/system_param.h>
#include <psp2/kernel/processmgr.h>

#include <vita2d.h>

#include "gui.h"
#include "utils.h"
#include "browser.h"
#include "init.h"
#include "config.h"
#include "file.h"

#define STATUS_BAR_PADDING_T 10
#define STATUS_BAR_PADDING_L 10
#define STATUS_BAR_BG_COLOR COLOR_SET_ALPHA(0x1F1F1F, 0x4F)

#define SCROLL_BAR_MIN_HEIGHT 4
#define SCROLL_BAR_COLOR COLOR_SET_ALPHA(LITEGRAY, 0x8F)
#define SCROLL_BAR_BG_COLOR COLOR_SET_ALPHA(DARKGRAY, 0x8F)

#define MAIN_BG_COLOR COLOR_SET_ALPHA(0x1F1F1F, 0x4F)

#define MAIN_TITLE APP_NAME_STR " " APP_VER_STR

char STR_BUTTON_ENTER[4], STR_BUTTON_CANCEL[4];

static vita2d_texture *wallpaper_tex = NULL;
static float wallpaper_x_scale, wallpaper_y_scale;

float MAIN_FREE_DRAW_WIDTH, MAIN_FREE_DRAW_HEIGHT;
float MAIN_FREE_DRAW_SX, MAIN_FREE_DRAW_DX, MAIN_FREE_DRAW_SY, MAIN_FREE_DRAW_DY;

static float status_bar_width, status_bar_height;

static void refreshLayout()
{
    status_bar_width = SCREEN_WIDTH;
    status_bar_height = GUI_getLineHeight() + STATUS_BAR_PADDING_T * 2;

    MAIN_FREE_DRAW_WIDTH = SCREEN_WIDTH;
    MAIN_FREE_DRAW_HEIGHT = SCREEN_HEIGHT - status_bar_height * 2;
    MAIN_FREE_DRAW_SX = 0;
    MAIN_FREE_DRAW_DX = MAIN_FREE_DRAW_SX + MAIN_FREE_DRAW_WIDTH;
    MAIN_FREE_DRAW_SY = status_bar_height;
    MAIN_FREE_DRAW_DY = MAIN_FREE_DRAW_SY + MAIN_FREE_DRAW_HEIGHT;
}

void GUI_drawTopStatusBar(char *title)
{
    int view_sx = 0;
    int view_dx = SCREEN_WIDTH;
    int view_sy = 0;

    // vita2d_draw_rectangle(view_sx, view_sy, status_bar_width, status_bar_height, STATUS_BAR_BG_COLOR);
    vita2d_draw_line(view_sx, view_sy + status_bar_height, view_sx + status_bar_width, view_sy + status_bar_height, WHITE);

    int sx = view_sx + STATUS_BAR_PADDING_L;
    int sy = view_sy + STATUS_BAR_PADDING_T;
    GUI_drawText(sx, sy, WHITE, title);

    sx = view_dx - STATUS_BAR_PADDING_L;
    if (!is_vitatv_model)
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
        float battery_x = sx - GUI_getTextWidth(battery_string);
        GUI_drawText(battery_x, sy, color, battery_string);
        sx = battery_x - STATUS_BAR_PADDING_L;
    }

    // Date & time
    SceDateTime time;
    sceRtcGetCurrentClock(&time, 0);

    char date_string[24];
    getDateString(date_string, date_format, &time);

    char time_string[16];
    getTimeString(time_string, time_format, &time);

    char string[64];
    snprintf(string, sizeof(string), "%s  %s", date_string, time_string);
    int date_time_x = sx - GUI_getTextWidth(string);
    GUI_drawText(date_time_x, sy, DEFALUT_FONT_COLOR, string);
}

void GUI_drawBottomStatusBar()
{
    int view_sx = 0;
    int view_sy = SCREEN_HEIGHT - status_bar_height;

    // vita2d_draw_rectangle(view_sx, view_sy, status_bar_width, status_bar_height, STATUS_BAR_BG_COLOR);
    vita2d_draw_line(view_sx, view_sy, view_sx + status_bar_width, view_sy, WHITE);

    int sx = view_sx + STATUS_BAR_PADDING_L;
    int sy = view_sy + STATUS_BAR_PADDING_T;
    GUI_drawTextf(sx, sy, WHITE, "Built on  %s by Yizhigai  %s", BUILD_DATE, REPOSITORY_ADDRESS);
}

static void drawMain()
{
    if (wallpaper_tex)
        vita2d_draw_texture_scale(wallpaper_tex, 0.0f, 0.0f, wallpaper_x_scale, wallpaper_y_scale);

    vita2d_draw_rectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MAIN_BG_COLOR);

    drawBrowser();
    GUI_drawTopStatusBar(MAIN_TITLE);
    GUI_drawBottomStatusBar();
}

static void controlMain()
{
    ctrlBrowser();
}

void GUI_main()
{
    readPad();
    GUI_startDrawing();
    drawMain();
    GUI_endDrawing();
    controlMain();
}

static int initImagesThreadCallback(SceSize args, void *argp)
{
    vita2d_texture *texture = vita2d_load_PNG_file(WALLPAPER_PNG_PATH);
    if (texture)
    {
        wallpaper_x_scale = SCREEN_WIDTH / (float)vita2d_texture_get_width(texture);
        wallpaper_y_scale = SCREEN_HEIGHT / (float)vita2d_texture_get_height(texture);
        wallpaper_tex = texture;
    }

    sceKernelExitDeleteThread(0);
    return 0;
}

static void initImagesThread()
{
    SceUID thid = sceKernelCreateThread("init_images_thread", initImagesThreadCallback, 0x10000100, 0x10000, 0, 0, NULL);
    if (thid >= 0)
        sceKernelStartThread(thid, 0, NULL);
}

static void deinitImages()
{
    if (wallpaper_tex)
    {
        vita2d_free_texture(wallpaper_tex);
        wallpaper_tex = NULL;
    }
}

void GUI_init()
{
    initImagesThread();
    GUI_initFonts();
    refreshLayout();
    initBrowser();
}

void GUI_deinit()
{
    GUI_deinitFonts();
    deinitImages();
}
