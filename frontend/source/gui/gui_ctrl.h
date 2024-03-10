#ifndef __M_GUI_CTRL_H__
#define __M_GUI_CTRL_H__

#include <psp2/ctrl.h>

enum ExtCtrlButtons
{
    /* SCE_CTRL_PSBUTTON = SCE_CTRL_INTERCEPTED is 17, we have 30 bitmask for set */
    EXT_CTRL_LEFT_ANLOG_UP = SCE_CTRL_INTERCEPTED << 1,     // 18
    EXT_CTRL_LEFT_ANLOG_RIGHT = SCE_CTRL_INTERCEPTED << 2,  // 19
    EXT_CTRL_LEFT_ANLOG_DOWN = SCE_CTRL_INTERCEPTED << 3,   // 20
    EXT_CTRL_LEFT_ANLOG_LEFT = SCE_CTRL_INTERCEPTED << 4,   // 21
    EXT_CTRL_RIGHT_ANLOG_UP = SCE_CTRL_INTERCEPTED << 5,    // 22
    EXT_CTRL_RIGHT_ANLOG_RIGHT = SCE_CTRL_INTERCEPTED << 6, // 23
    EXT_CTRL_RIGHT_ANLOG_DOWN = SCE_CTRL_INTERCEPTED << 7,  // 24
    EXT_CTRL_RIGHT_ANLOG_LEFT = SCE_CTRL_INTERCEPTED << 8,  // 25
};

enum PadButtons
{
    PAD_LEFT,
    PAD_UP,
    PAD_RIGHT,
    PAD_DOWN,
    PAD_CROSS,
    PAD_CIRCLE,
    PAD_SQUARE,
    PAD_TRIANGLE,
    PAD_L1,
    PAD_R1,
    PAD_L2,
    PAD_R2,
    PAD_L3,
    PAD_R3,
    PAD_SELECT,
    PAD_START,
    PAD_PSBUTTON,
    PAD_ENTER,
    PAD_CANCEL,
    PAD_LEFT_ANALOG_UP,
    PAD_LEFT_ANALOG_DOWN,
    PAD_LEFT_ANALOG_LEFT,
    PAD_LEFT_ANALOG_RIGHT,
    PAD_RIGHT_ANALOG_UP,
    PAD_RIGHT_ANALOG_DOWN,
    PAD_RIGHT_ANALOG_LEFT,
    PAD_RIGHT_ANALOG_RIGHT,
    PAD_N_BUTTONS
};

#define ANALOG_CENTER 128
#define ANALOG_THRESHOLD 64
#define ANALOG_SENSITIVITY 16

#define N_CTRL_PORTS 4

#define DISABLE_PSBUTTON_HOLD_MICROS (500000llu) // 0.5 second (当按住到达这个时间时会禁用ps键)

typedef uint8_t Pad[PAD_N_BUTTONS];

extern Pad old_pad, current_pad, pressed_pad, released_pad, hold_pad, hold2_pad;
extern Pad hold_count, hold2_count;

void GUI_ReadPad();
void GUI_CleanPad();

void GUI_SetControlEnabled(int enable); // 设置禁用后会一直禁用获取不到键值，需要手动去再开启
int GUI_IsControlEnabled();
void GUI_SetPsbuttonEnabled(int enable); // 只是内部传递，通过ReadPad仍能获取键值，接收判断为GUI_IsPsbuttonEnabled
int GUI_IsPsbuttonEnabled();

#endif