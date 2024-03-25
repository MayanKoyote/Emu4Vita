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

static int gui_async_draw = 0;
static SceUID gui_draw_thid = -1;
static GUI_Texture *gui_images[N_GUI_IMAGES] = {0};

extern int GUI_BeforeDrawActivity();
extern int GUI_DrawActivity();
extern int GUI_AfterDrawActivity();
extern int GUI_CtrlActivity();
extern int GUI_EventActivity();

extern int GUI_BeforeDrawWindow();
extern int GUI_DrawWindow();
extern int GUI_AfterDrawWindow();
extern int GUI_CtrlWindow();
extern int GUI_EventWindow();

extern int GUI_BeforeDrawToast();
extern int GUI_DrawToast();
extern int GUI_AfterDrawToast();

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

void GUI_SetImage(GUI_ImageId id, GUI_Texture *texture)
{
    if (id >= 0 && id < N_GUI_IMAGES)
    {
        if (gui_images[id])
            GUI_DestroyTexture(gui_images[id]);
        gui_images[id] = texture;
    }
}

GUI_Texture *GUI_GetImage(GUI_ImageId id)
{
    if (id >= 0 && id < N_GUI_IMAGES)
        return gui_images[id];
    return NULL;
}

static void GUI_BeforeDrawMain()
{
    GUI_BeforeDrawActivity();
    GUI_BeforeDrawWindow();
    GUI_BeforeDrawToast();
}

static void GUI_DrawMain()
{
    GUI_LockDrawMutex();
    GUI_StartDrawing(NULL);
    GUI_DrawActivity();
    GUI_DrawWindow();
    GUI_DrawToast();
    GUI_EndDrawing();
    GUI_UnlockDrawMutex();
}

static void GUI_AfterDrawMain()
{
    GUI_AfterDrawActivity();
    GUI_AfterDrawWindow();
    GUI_AfterDrawToast();
}

static void onHomeKeyEvent()
{
    if (released_pad[PAD_PSBUTTON])
    {
        if (GUI_IsHomeKeyEnabled() && GUI_IsHomeEventEnabled() && !GUI_IsInMainActivity())
        {
            GUI_SetHomeKeyEnabled(0);
            GUI_CloseWindowEx(TYPE_WINDOW_CLOSE_ALL, NULL);
            GUI_BackToMainActivity();
        }
    }
}

static void GUI_CtrlMain()
{
    GUI_ReadPad();
    onHomeKeyEvent();
    if (GUI_GetWindowCount() > 0)
        GUI_CtrlWindow();
    else
        GUI_CtrlActivity();
}

static void GUI_EventMain()
{
    GUI_EventActivity();
    GUI_EventWindow();
}

void GUI_Run()
{
    if (!gui_async_draw)
    {
        GUI_BeforeDrawMain();
        GUI_DrawMain();
        GUI_AfterDrawMain();
    }
    GUI_CtrlMain();
    GUI_EventMain();
    AutoUnlockQuickMenu();
}

static int guiDrawThreadEntry(SceSize args, void *argp)
{
    while (gui_async_draw)
    {
        GUI_BeforeDrawMain();
        GUI_DrawMain();
        GUI_AfterDrawMain();
    }

    sceKernelExitThread(0);
    return 0;
}

int GUI_StartAsyncDraw()
{
    int ret = 0;

    if (gui_draw_thid < 0)
    {
        ret = gui_draw_thid = sceKernelCreateThread("gui_draw_thread", guiDrawThreadEntry, 0x10000100, 0x10000, 0, 0, NULL);
        if (gui_draw_thid >= 0)
        {
            gui_async_draw = 1;
            ret = sceKernelStartThread(gui_draw_thid, 0, NULL);
            if (ret < 0)
            {
                gui_async_draw = 0;
                sceKernelDeleteThread(gui_draw_thid);
                gui_draw_thid = -1;
            }
        }
    }

    return ret;
}

void GUI_FinishAsyncDraw()
{
    if (gui_draw_thid >= 0)
    {
        gui_async_draw = 0;
        sceKernelWaitThreadEnd(gui_draw_thid, NULL, NULL);
        sceKernelDeleteThread(gui_draw_thid);
        gui_draw_thid = -1;
    }
}
