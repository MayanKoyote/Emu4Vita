#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <psp2/kernel/threadmgr.h>

#include "gui.h"
#include "utils.h"
#include "app.h"

static SceKernelLwMutexWork gui_draw_mutex = {0};
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
    if (released_pad[PAD_HOME])
    {
        if (GUI_IsHomeKeyEnabled() && GUI_IsHomeEventEnabled() && !GUI_IsInMainActivity())
        {
            GUI_SetHomeKeyEnabled(0);
            GUI_CloseAllWindows();
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

void GUI_RunMain()
{
    GUI_BeforeDrawMain();
    GUI_DrawMain();
    GUI_AfterDrawMain();
    GUI_CtrlMain();
    GUI_EventMain();
    AutoUnlockQuickMenu();
}

int GUI_LockDrawMutex()
{
    return sceKernelLockLwMutex(&gui_draw_mutex, 1, NULL);
}

int GUI_UnlockDrawMutex()
{
    return sceKernelUnlockLwMutex(&gui_draw_mutex, 1);
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
