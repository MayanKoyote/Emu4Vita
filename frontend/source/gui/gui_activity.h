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
    int title;                                          // Title
    GUI_ButtonInstruction *button_instructions;         // Button instructions
    GUI_Texture *wallpaper;                             // Wallpaper
    int no_statusbar;                                   // Disable draw statusbar
    int disable_home_event;                             // Disable home event
    int (*onStart)(struct GUI_Activity *activity);      // Start callback
    int (*onFinish)(struct GUI_Activity *activity);     // Finish callback
    int (*onBeforeDraw)(struct GUI_Activity *activity); // Before draw callback
    int (*onDraw)(struct GUI_Activity *activity);       // Draw callback
    int (*onAfterDraw)(struct GUI_Activity *activity);  // After draw callback
    int (*onCtrl)(struct GUI_Activity *activity);       // Ctrl callback
    int (*onEvent)(struct GUI_Activity *activity);      // Event callback
    void *userdata;                                     // User data
} GUI_Activity;

int GUI_InitActivity();
int GUI_DeinitActivity();

int GUI_StartActivity(GUI_Activity *activity);
int GUI_FinishActivity(GUI_Activity *activity);
int GUI_FinishAllActivities();
int GUI_FinishOtherActivities();
int GUI_BackToMainActivity();
int GUI_IsInMainActivity();
int GUI_IsHomeEventEnabled();

int GUI_GetActivityCount();
int GUI_GetActivityLayoutPosition(GUI_Activity *activity, int *x, int *y);
int GUI_GetActivityAvailableSize(GUI_Activity *activity, int *w, int *h);

#endif
