#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <psp2/kernel/processmgr.h>

#include "gui/gui.h"
#include "emu/emu.h"
#include "config.h"
#include "lang.h"

static int onStartActivity(GUI_Activity *activity);
static int onFinishActivity(GUI_Activity *activity);
static int onDrawActivity(GUI_Activity *activity);
static int onCtrlActivity(GUI_Activity *activity);
static int onEventActivity(GUI_Activity *activity);

static uint64_t micros_per_frame = 0;
static uint64_t last_frame_micros = 0;

GUI_Activity emu_activity = {
    LANG_NULL,        // Title
    NULL,             // Button instructions
    NULL,             // Wallpaper
    1,                // Disable draw statusbar
    onStartActivity,  // Start callback
    onFinishActivity, // Finish callback
    onDrawActivity,   // Draw callback
    onCtrlActivity,   // Ctrl callback
    onEventActivity,  // Event callback
    NULL,             // User data
};

static void checkFrameDelay()
{
    uint64_t cur_micros = sceKernelGetProcessTimeWide();
    uint64_t interval_micros = cur_micros - last_frame_micros;
    if (interval_micros < micros_per_frame)
    {
        uint64_t delay_micros = micros_per_frame - interval_micros;
        sceKernelDelayThread(delay_micros);
        last_frame_micros = cur_micros + delay_micros;
    }
    else
    {
        last_frame_micros = cur_micros;
    }
}

static int onStartActivity(GUI_Activity *activity)
{
    GUI_SetBackToMainActivityEnabled(0);
    return 0;
}

static int onFinishActivity(GUI_Activity *activity)
{
    GUI_SetBackToMainActivityEnabled(1);
    return 0;
}

static int onDrawActivity(GUI_Activity *activity)
{
    Emu_DrawVideo();

    if (Emu_IsGameRunning())
    {
        Emu_DrawVideoWidgets();
        checkFrameDelay();
    }

    return 0;
}

static int onCtrlActivity(GUI_Activity *activity)
{
    return 0;
}

static int onEventActivity(GUI_Activity *activity)
{
    if (Emu_IsGameRunning())
        Emu_RunGame();

    Emu_EventVideo();

    return 0;
}

void Emu_SetMicrosPerFrame(uint64_t micros)
{
    micros_per_frame = micros;
}

uint64_t Emu_GetMicrosPerFrame()
{
    return micros_per_frame;
}
