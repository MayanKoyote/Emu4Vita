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
static int onBeforeDrawActivity(GUI_Activity *activity);
static int onDrawActivity(GUI_Activity *activity);
static int onAfterDrawActivity(GUI_Activity *activity);
static int onCtrlActivity(GUI_Activity *activity);
static int onEventActivity(GUI_Activity *activity);

GUI_Activity emu_activity = {
    LANG_NULL,            // Title
    NULL,                 // Button instructions
    NULL,                 // Wallpaper
    1,                    // Disable draw statusbar
    1,                    // Disable home event
    onStartActivity,      // Start callback
    onFinishActivity,     // Finish callback
    onBeforeDrawActivity, // Before draw callback
    onDrawActivity,       // Draw callback
    onAfterDrawActivity,  // After callback
    onCtrlActivity,       // Ctrl callback
    onEventActivity,      // Event callback
    NULL,                 // User data
};

static int onStartActivity(GUI_Activity *activity)
{
    return 0;
}

static int onFinishActivity(GUI_Activity *activity)
{
    return 0;
}

static int onBeforeDrawActivity(GUI_Activity *activity)
{
    if (Emu_IsGameRunning())
        Emu_WaitVideoSema();

    return 0;
}

static int onDrawActivity(GUI_Activity *activity)
{
    Emu_DrawVideo();

    if (Emu_IsGameRunning())
        Emu_DrawVideoWidgets();

    return 0;
}

static int onAfterDrawActivity(GUI_Activity *activity)
{
    if (!Emu_IsGameRunning())
        GUI_SignalDrawSema();

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
    else
        GUI_WaitDrawSema();

    Emu_EventVideo();

    return 0;
}
