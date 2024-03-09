#ifndef __M_GUI_ACTIVITY_H__
#define __M_GUI_ACTIVITY_H__

#include "gui_lib.h"

typedef struct
{
    int button;
    int instruction;
    int visibility;
} GUI_ButtonInstruction;

typedef struct GUI_Activity
{
    int title;                                      // Title
    GUI_ButtonInstruction *button_instructions;     // Button instructions
    GUI_Texture *wallpaper;                         // Wallpaper
    int nostatusbar;                                // Disable draw statusbar
    int (*onStart)(struct GUI_Activity *activity);  // Start callback
    int (*onFinish)(struct GUI_Activity *activity); // Finish callback
    int (*onDraw)(struct GUI_Activity *activity);   // Draw callback
    int (*onCtrl)(struct GUI_Activity *activity);   // Ctrl callback
    int (*onEvent)(struct GUI_Activity *activity);  // Event callback
    void *userdata;                                 // User data
} GUI_Activity;

int GUI_StartActivity(GUI_Activity *activity);
int GUI_FinishActivity(GUI_Activity *activity);
int GUI_BackToMainActivity();
int GUI_IsInMainActivity();

int GUI_GetActivityCount();
GUI_Activity *GUI_GetCurrentActivity();
GUI_Activity *GUI_GetPrevActivity(GUI_Activity *activity);
GUI_Activity *GUI_GetNextActivity(GUI_Activity *activity);

int GUI_GetActivityLayoutPosition(GUI_Activity *activity, int *x, int *y);
int GUI_GetActivityAvailableSize(GUI_Activity *activity, int *w, int *h);

#endif
