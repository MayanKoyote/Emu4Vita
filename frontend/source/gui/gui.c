#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <psp2/kernel/threadmgr.h>
#include <psp2/system_param.h>
#include <psp2/power.h>
#include <psp2/ctrl.h>

#include "gui.h"
#include "utils.h"
#include "lang.h"
#include "init.h"

static int gui_thread_run = 0;
static SceUID gui_thread_run_thid = -1;
static int gui_back_to_main_activity_enabled = 1;
static GUI_Texture *wallpaper_texture = NULL, *splash_texture = NULL;
static GUI_Texture *checkbox_on_texture = NULL, *checkbox_off_texture = NULL;
static GUI_Texture *radiobutton_on_texture = NULL, *radiobutton_off_texture = NULL;

extern int GUI_DrawWindow();
extern int GUI_CtrlWindow();
extern int GUI_EventWindow();

extern int GUI_DrawActivity();
extern int GUI_CtrlActivity();
extern int GUI_EventActivity();

extern int GUI_DrawToast();
extern int GUI_EventToast();

void GUI_DrawVerticalScrollbar(int track_x, int track_y, int track_height, int max_len, int draw_len, int scroll_len, int draw_track)
{
    if (track_height <= 0 || draw_len <= 0 || max_len <= draw_len)
        return;

    // Draw scroll track
    if (draw_track)
        GUI_DrawFillRectangle(track_x, track_y, GUI_DEF_SCROLLBAR_SIZE, track_height, GUI_DEF_SCROLLBAR_COLOR_TRACK);

    // Draw scroll thumb
    float size_per_item = (float)track_height / (float)max_len;
    int thumb_height = (float)draw_len * size_per_item;
    thumb_height = MAX(thumb_height, 1); // Fix

    int min_y = track_y;
    int max_y = track_y + track_height - thumb_height;
    int thumb_y = track_y + (float)scroll_len * size_per_item;
    if (thumb_y < min_y)
        thumb_y = min_y;
    else if (thumb_y > max_y)
        thumb_y = max_y;

    GUI_DrawFillRectangle(track_x, thumb_y, GUI_DEF_SCROLLBAR_SIZE, thumb_height, GUI_DEF_SCROLLBAR_COLOR_THUMB);
}

void GUI_SetBackToMainActivityEnabled(int enabled)
{
    gui_back_to_main_activity_enabled = enabled;
}

void GUI_SetDefaultWallpaper(GUI_Texture *texture)
{
    if (wallpaper_texture)
        GUI_DestroyTexture(wallpaper_texture);
    wallpaper_texture = texture;
}

void GUI_SetDefaultSplash(GUI_Texture *texture)
{
    if (splash_texture)
        GUI_DestroyTexture(splash_texture);
    splash_texture = texture;
}

void GUI_SetCheckBoxTexture(GUI_Texture *on_texture, GUI_Texture *off_texture)
{
    if (checkbox_on_texture)
        GUI_DestroyTexture(checkbox_on_texture);
    if (checkbox_off_texture)
        GUI_DestroyTexture(checkbox_off_texture);
    checkbox_on_texture = on_texture;
    checkbox_off_texture = off_texture;
}

void GUI_SetRadioButtonTexture(GUI_Texture *on_texture, GUI_Texture *off_texture)
{
    if (radiobutton_on_texture)
        GUI_DestroyTexture(radiobutton_on_texture);
    if (radiobutton_off_texture)
        GUI_DestroyTexture(radiobutton_off_texture);
    radiobutton_on_texture = on_texture;
    radiobutton_off_texture = off_texture;
}

GUI_Texture *GUI_GetDefaultWallpaper()
{
    return wallpaper_texture;
}

GUI_Texture *GUI_GetDefaultSplash()
{
    return splash_texture;
}

void GUI_GetCheckBoxTexture(GUI_Texture **on_texture, GUI_Texture **off_texture)
{
    if (on_texture)
        *on_texture = checkbox_on_texture;
    if (off_texture)
        *off_texture = checkbox_off_texture;
}

void GUI_GetRadioButtonTexture(GUI_Texture **on_texture, GUI_Texture **off_texture)
{
    if (on_texture)
        *on_texture = radiobutton_on_texture;
    if (off_texture)
        *off_texture = radiobutton_off_texture;
}

static void GUI_DrawMain()
{
    GUI_LockDraw();
    GUI_StartDrawing(NULL);
    GUI_DrawActivity();
    GUI_DrawWindow();
    GUI_DrawToast();
    GUI_EndDrawing();
    GUI_UnlockDraw();
}

static void onHomeButtonEvent()
{
    if (released_pad[PAD_PSBUTTON])
    {
        if (GUI_IsPsbuttonEnabled())
        {
            if (gui_back_to_main_activity_enabled && !GUI_IsInMainActivity())
            {
                GUI_SetPsbuttonEnabled(0);
                GUI_CloseWindowEx(TYPE_WINDOW_CLOSE_ALL, NULL);
                GUI_BackToMainActivity();
            }
        }
    }
}

static void GUI_CtrlMain()
{
    GUI_ReadPad();
    onHomeButtonEvent();
    if (GUI_GetWindowCount() > 0)
        GUI_CtrlWindow();
    else
        GUI_CtrlActivity();
}

static void GUI_EventMain()
{
    GUI_EventActivity();
    GUI_EventWindow();
    GUI_EventToast();
}

static void GUI_RunBase()
{
    GUI_DrawMain();
    GUI_CtrlMain();
    GUI_EventMain();
    AutoUnlockQuickMenu();
}

void GUI_Run()
{
    if (gui_thread_run && gui_thread_run_thid >= 0)
        sceKernelWaitThreadEnd(gui_thread_run_thid, NULL, NULL);
    GUI_RunBase();
}

static int guiRunThreadEntry(SceSize args, void *argp)
{
    while (gui_thread_run)
        GUI_RunBase();

    sceKernelExitDeleteThread(0);
    return 0;
}

int GUI_StartThreadRun()
{
    int ret = 0;

    if (gui_thread_run_thid < 0)
    {
        ret = gui_thread_run_thid = sceKernelCreateThread("gui_run_thread", guiRunThreadEntry, 0x10000100, 0x10000, 0, 0, NULL);
        if (gui_thread_run_thid >= 0)
        {
            gui_thread_run = 1;
            ret = sceKernelStartThread(gui_thread_run_thid, 0, NULL);
            if (ret < 0)
            {
                sceKernelDeleteThread(gui_thread_run_thid);
                gui_thread_run_thid = -1;
                gui_thread_run = 0;
            }
        }
    }

    return ret;
}

void GUI_ExitThreadRun()
{
    gui_thread_run = 0;
    if (gui_thread_run_thid >= 0)
    {
        sceKernelWaitThreadEnd(gui_thread_run_thid, NULL, NULL);
        sceKernelDeleteThread(gui_thread_run_thid);
        gui_thread_run_thid = -1;
    }
}

int GUI_IsInThreadRun()
{
    return gui_thread_run;
}
