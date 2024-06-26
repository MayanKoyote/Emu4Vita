#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <psp2/kernel/processmgr.h>
#include <psp2/ctrl.h>
#include <psp2/touch.h>

#include "setting/setting.h"
#include "gui/gui.h"
#include "emu/emu.h"
#include "utils.h"
#include "config.h"
#include "app.h"

typedef struct
{
    int mapping_key;
    int sx;
    int sy;
    int dx;
    int dy;
} TouchMap;

typedef struct
{
    uint32_t *enable;
    TouchMap *maps;
    int n_maps;
} TouchOption;

typedef struct
{
    uint32_t *value;
    int local_key;
    uint8_t old_presseds[N_CTRL_PORTS];
    uint32_t hold_count[N_CTRL_PORTS];
} EmuKeyOption;

typedef struct
{
    uint32_t *value;
    EmuGameEventAction action;
    uint8_t old_presseds[N_CTRL_PORTS];
    uint32_t hold_count[N_CTRL_PORTS];
} HotKeyOption;

#define RETRO_N_PAD_BUTTONS 16

#define FRONT_TOUCH_WIDTH 200
#define FRONT_TOUCH_HEIGHT 200
#define BACK_TOUCH_WIDTH 400
#define BACK_TOUCH_HEIGHT 300
#define BACK_TOUCH_PADDING_L 0

TouchMap front_touch_maps[] = {
    {GUI_CTRL_BUTTON_L2, 0, 0, FRONT_TOUCH_WIDTH, FRONT_TOUCH_HEIGHT},
    {GUI_CTRL_BUTTON_R2, GUI_SCREEN_WIDTH - FRONT_TOUCH_WIDTH, 0, GUI_SCREEN_WIDTH, FRONT_TOUCH_HEIGHT},
    {GUI_CTRL_BUTTON_L3, 0, GUI_SCREEN_HEIGHT - FRONT_TOUCH_HEIGHT, FRONT_TOUCH_WIDTH, GUI_SCREEN_HEIGHT},
    {GUI_CTRL_BUTTON_R3, GUI_SCREEN_WIDTH - FRONT_TOUCH_WIDTH, GUI_SCREEN_HEIGHT - FRONT_TOUCH_HEIGHT, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT},
};

TouchMap back_touch_maps[] = {
    {GUI_CTRL_BUTTON_L2, BACK_TOUCH_PADDING_L, 0, BACK_TOUCH_PADDING_L + BACK_TOUCH_WIDTH, BACK_TOUCH_HEIGHT},
    {GUI_CTRL_BUTTON_R2, GUI_SCREEN_WIDTH - BACK_TOUCH_PADDING_L - BACK_TOUCH_WIDTH, 0, GUI_SCREEN_WIDTH, BACK_TOUCH_HEIGHT},
    {GUI_CTRL_BUTTON_L3, BACK_TOUCH_PADDING_L, GUI_SCREEN_HEIGHT - BACK_TOUCH_HEIGHT, BACK_TOUCH_PADDING_L + BACK_TOUCH_WIDTH, GUI_SCREEN_HEIGHT},
    {GUI_CTRL_BUTTON_R3, GUI_SCREEN_WIDTH - BACK_TOUCH_PADDING_L - BACK_TOUCH_WIDTH, GUI_SCREEN_HEIGHT - BACK_TOUCH_HEIGHT, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT},
};

TouchOption touch_options[2] = {
    {&control_config.front_touch_pad, front_touch_maps, sizeof(front_touch_maps) / sizeof(TouchMap)},
    {&control_config.back_touch_pad, back_touch_maps, sizeof(back_touch_maps) / sizeof(TouchMap)},
};
#define N_TOUCH_OPTIONS (sizeof(touch_options) / sizeof(TouchOption))

EmuKeyOption emu_key_options[] = {
    {&control_config.button_left, GUI_CTRL_BUTTON_LEFT, {0}, {0}},
    {&control_config.button_up, GUI_CTRL_BUTTON_UP, {0}, {0}},
    {&control_config.button_right, GUI_CTRL_BUTTON_RIGHT, {0}, {0}},
    {&control_config.button_down, GUI_CTRL_BUTTON_DOWN, {0}, {0}},
    {&control_config.button_a, GUI_CTRL_BUTTON_A, {0}, {0}},
    {&control_config.button_b, GUI_CTRL_BUTTON_B, {0}, {0}},
    {&control_config.button_x, GUI_CTRL_BUTTON_X, {0}, {0}},
    {&control_config.button_y, GUI_CTRL_BUTTON_Y, {0}, {0}},
    {&control_config.button_select, GUI_CTRL_BUTTON_SELECT, {0}, {0}},
    {&control_config.button_start, GUI_CTRL_BUTTON_START, {0}, {0}},
    {&control_config.button_l1, GUI_CTRL_BUTTON_L1, {0}, {0}},
    {&control_config.button_r1, GUI_CTRL_BUTTON_R1, {0}, {0}},
    {&control_config.button_l2, GUI_CTRL_BUTTON_L2, {0}, {0}},
    {&control_config.button_r2, GUI_CTRL_BUTTON_R2, {0}, {0}},
    {&control_config.button_l3, GUI_CTRL_BUTTON_L3, {0}, {0}},
    {&control_config.button_r3, GUI_CTRL_BUTTON_R3, {0}, {0}},
    {&control_config.left_analog_left, GUI_CTRL_BUTTON_LEFT_ANLOG_LEFT, {0}, {0}},
    {&control_config.left_analog_up, GUI_CTRL_BUTTON_LEFT_ANLOG_UP, {0}, {0}},
    {&control_config.left_analog_right, GUI_CTRL_BUTTON_LEFT_ANLOG_RIGHT, {0}, {0}},
    {&control_config.left_analog_down, GUI_CTRL_BUTTON_LEFT_ANLOG_DOWN, {0}, {0}},
    {&control_config.right_analog_left, GUI_CTRL_BUTTON_RIGHT_ANLOG_LEFT, {0}, {0}},
    {&control_config.right_analog_up, GUI_CTRL_BUTTON_RIGHT_ANLOG_UP, {0}, {0}},
    {&control_config.right_analog_right, GUI_CTRL_BUTTON_RIGHT_ANLOG_RIGHT, {0}, {0}},
    {&control_config.right_analog_down, GUI_CTRL_BUTTON_RIGHT_ANLOG_DOWN, {0}, {0}},
};
#define N_EMU_KEY_OPTIONS (sizeof(emu_key_options) / sizeof(EmuKeyOption))

HotKeyOption hot_key_options[] = {
    {&hotkey_config.hk_save_state, EMU_GAME_EVENT_ACTION_SAVE_STATE, {0}, {0}},
    {&hotkey_config.hk_load_state, EMU_GAME_EVENT_ACTION_LOAD_STATE, {0}, {0}},
    {&hotkey_config.hk_speed_up, EMU_GAME_EVENT_ACTION_SPEED_UP, {0}, {0}},
    {&hotkey_config.hk_speed_down, EMU_GAME_EVENT_ACTION_SPEED_DOWN, {0}, {0}},
    {&hotkey_config.hk_rewind_game, EMU_GAME_EVENT_ACTION_REWIND, {0}, {0}},
    {&hotkey_config.hk_controller_up, EMU_GAME_EVENT_ACTION_CONTROLLER_PORT_UP, {0}, {0}},
    {&hotkey_config.hk_controller_down, EMU_GAME_EVENT_ACTION_CONTROLLER_PORT_DOWN, {0}, {0}},
    {&hotkey_config.hk_exit_game, EMU_GAME_EVENT_ACTION_EXIT, {0}, {0}},
};
#define N_HOT_KEY_OPTIONS (sizeof(hot_key_options) / sizeof(HotKeyOption))

static uint8_t homekey_old_pressed[N_CTRL_PORTS];
static uint64_t homekey_disable_micros[N_CTRL_PORTS];
static uint32_t emu_mapping_keys[N_CTRL_PORTS]; // bitmask
static int input_okay = 0;

static void cleanInputKeys()
{
    int i, j;

    for (i = 0; i < N_EMU_KEY_OPTIONS; i++)
    {
        for (j = 0; j < N_CTRL_PORTS; j++)
        {
            emu_key_options[i].old_presseds[j] = 0;
            emu_key_options[i].hold_count[j] = 0;
        }
    }

    for (i = 0; i < N_HOT_KEY_OPTIONS; i++)
    {
        for (j = 0; j < N_CTRL_PORTS; j++)
        {
            hot_key_options[i].old_presseds[j] = 0;
            hot_key_options[i].hold_count[j] = 0;
        }
    }

    for (i = 0; i < N_CTRL_PORTS; i++)
    {
        homekey_old_pressed[i] = 0;
    }
}

static void TouchToButtons(uint32_t *buttons)
{
    if (IsVitatvModel())
        return;

    SceTouchData touch_data;
    int i, j, k;
    int x, y;
    for (i = 0; i < N_TOUCH_OPTIONS; i++)
    {
        TouchOption *option = &touch_options[i];
        if (!*(option->enable))
            continue;

        memset(&touch_data, 0, sizeof(SceTouchData));
        if (sceTouchPeek(i, &touch_data, 1) < 0)
            continue;

        TouchMap *maps = option->maps;
        for (j = 0; j < touch_data.reportNum; j++)
        {
            x = lerp(touch_data.report[j].x, 1919, GUI_SCREEN_WIDTH);
            y = lerp(touch_data.report[j].y, 1087, GUI_SCREEN_HEIGHT);
            for (k = 0; k < option->n_maps; k++)
            {
                if ((x >= maps[k].sx) && (x <= maps[k].dx) && (y >= maps[k].sy) && (y <= maps[k].dy))
                    *buttons |= maps[k].mapping_key;
            }
        }
    }
}

static void AnalogToButtons(uint8_t lanalog_x, uint8_t lanalog_y, uint8_t ranalog_x, uint8_t ranalog_y, uint32_t *buttons)
{
    if (lanalog_x < ANALOG_CENTER - ANALOG_THRESHOLD)
        *buttons |= GUI_CTRL_BUTTON_LEFT_ANLOG_LEFT;
    else if (lanalog_x > ANALOG_CENTER + ANALOG_THRESHOLD)
        *buttons |= GUI_CTRL_BUTTON_LEFT_ANLOG_RIGHT;
    if (lanalog_y < ANALOG_CENTER - ANALOG_THRESHOLD)
        *buttons |= GUI_CTRL_BUTTON_LEFT_ANLOG_UP;
    else if (lanalog_y > ANALOG_CENTER + ANALOG_THRESHOLD)
        *buttons |= GUI_CTRL_BUTTON_LEFT_ANLOG_DOWN;

    if (ranalog_x < ANALOG_CENTER - ANALOG_THRESHOLD)
        *buttons |= GUI_CTRL_BUTTON_RIGHT_ANLOG_LEFT;
    else if (ranalog_x > ANALOG_CENTER + ANALOG_THRESHOLD)
        *buttons |= GUI_CTRL_BUTTON_RIGHT_ANLOG_RIGHT;
    if (ranalog_y < ANALOG_CENTER - ANALOG_THRESHOLD)
        *buttons |= GUI_CTRL_BUTTON_RIGHT_ANLOG_UP;
    else if (ranalog_y > ANALOG_CENTER + ANALOG_THRESHOLD)
        *buttons |= GUI_CTRL_BUTTON_RIGHT_ANLOG_DOWN;
}

static void LocalKeyToEmuKey(EmuKeyOption *option, int local_port, int map_port, uint32_t buttons)
{
    uint8_t cur_pressed = ((buttons & option->local_key) != 0);
    uint8_t old_pressed = option->old_presseds[local_port];
    option->old_presseds[local_port] = cur_pressed;

    if (cur_pressed)
    {
        int hold_count = ++option->hold_count[local_port];
        uint32_t config_key = *(option->value);
        int trubo = config_key & TURBO_BITMASK_KEY;
        uint32_t map_key = config_key & 0x0FFFFFFF;
        if (!map_key)
            return;

        if (!trubo || !old_pressed || (hold_count % (control_config.turbo_delay + 1) == 0))
            emu_mapping_keys[map_port] |= map_key;
    }
    else
    {
        option->hold_count[local_port] = 0;
    }
}

static int onHotKeyEvent(int port, uint32_t buttons)
{
    uint8_t cur_pressed;
    uint8_t old_pressed;
    HotKeyOption *option;
    uint32_t config_key;
    uint32_t map_key;
    int trubo;
    int hotkey_pressed;

    int i;
    for (i = 0; i < N_HOT_KEY_OPTIONS; i++)
    {
        hotkey_pressed = 0;
        option = &hot_key_options[i];
        config_key = *(option->value);
        trubo = config_key & TURBO_BITMASK_KEY;
        map_key = config_key & 0x0FFFFFFF;
        if (!map_key)
            continue;

        cur_pressed = ((buttons & map_key) == map_key);
        old_pressed = option->old_presseds[port];
        option->old_presseds[port] = cur_pressed;

        if (cur_pressed)
        {
            ++option->hold_count[port];
            if (trubo)
            {
                if (!old_pressed)
                {
                    hotkey_pressed = 1;
                }
                else if (option->hold_count[port] >= 10)
                {
                    hotkey_pressed = 1;
                    option->hold_count[port] = 6; // 第二次开始加快
                }
            }
        }
        else
        {
            option->hold_count[port] = 0;
            if (!trubo && old_pressed) // 连发时取消按键弹出事件，连发是(!old && cur)，非连发是(!cur && old)
                hotkey_pressed = 1;
        }

        if (hotkey_pressed)
        {
            Emu_SetGameEventAction(option->action);
            if (config_key & GUI_CTRL_BUTTON_HOME)
                GUI_SetHomeKeyEnabled(0);
            return 1;
        }
    }

    return 0;
}

static int onHomeKeyEvent(int prot, uint32_t buttons)
{
    uint8_t cur_pressed = ((buttons & GUI_CTRL_BUTTON_HOME) == GUI_CTRL_BUTTON_HOME);
    uint8_t old_pressed = homekey_old_pressed[prot];
    homekey_old_pressed[prot] = cur_pressed;

    if (cur_pressed)
    {
        if (!old_pressed)
            homekey_disable_micros[prot] = sceKernelGetProcessTimeWide() + DISABLE_PSBUTTON_HOLD_MICROS;
        else if (sceKernelGetProcessTimeWide() >= homekey_disable_micros[prot])
            GUI_SetHomeKeyEnabled(0);
        return 1;
    }
    else
    {
        if (old_pressed)
        {
            if (GUI_IsHomeKeyEnabled())
            {
                float speed = Emu_GetRunSpeed();
                if (speed != 1.0f)
                {
                    speed = 1.0f;
                    Emu_SetRunSpeed(speed);
                }
                else
                {
                    GUI_SetHomeKeyEnabled(0);
                    Emu_SetGameEventAction(EMU_GAME_EVENT_ACTION_OPEN_SETTING_MENU);
                }
                return 1;
            }
        }
        else
        {
            GUI_SetHomeKeyEnabled(1);
        }
    }

    return 0;
}

void Emu_PollInput()
{
    SceCtrlData ctrl_data;
    int read_port, local_port, map_port;
    int i;

    memset(emu_mapping_keys, 0, sizeof(emu_mapping_keys));

    if (!GUI_IsControlEnabled())
        return;

    for (local_port = 0; local_port < N_CTRL_PORTS; local_port++)
    {
        if (local_port == 0)
        {
            read_port = 0;
            map_port = control_config.controller_port;
        }
        else
        {
            read_port = local_port + 1;
            map_port = local_port;
        }

        memset(&ctrl_data, 0, sizeof(SceCtrlData));
        if (sceCtrlPeekBufferPositiveExt2(read_port, &ctrl_data, 1) < 0)
            continue;

        ctrl_data.buttons &= GUI_CTRL_BUTTONS_BITMASK;

        if (local_port == 0)
            TouchToButtons(&ctrl_data.buttons);
        AnalogToButtons(ctrl_data.lx, ctrl_data.ly, ctrl_data.rx, ctrl_data.ry, &ctrl_data.buttons);

        if (onHotKeyEvent(local_port, ctrl_data.buttons))
            return;

        if (onHomeKeyEvent(local_port, ctrl_data.buttons))
            return;

        for (i = 0; i < N_EMU_KEY_OPTIONS; i++)
            LocalKeyToEmuKey(&emu_key_options[i], local_port, map_port, ctrl_data.buttons);
    }
}

int Emu_InitInput()
{
    cleanInputKeys();
    input_okay = 1;
    return 0;
}

int Emu_DeinitInput()
{
    input_okay = 0;
    return 0;
}

void Retro_InputPollCallback()
{
    // Do nothing, we use Emu_PollInput() to poll the input, because sometimes the retro core
    // do not invoke this callback, that will make us failed to open the menu in game running.
}

int16_t Retro_InputStateCallback(unsigned port, unsigned device, unsigned index, unsigned id)
{
    if (!input_okay || port >= N_CTRL_PORTS)
        return 0;

    int16_t res = 0;

    if (device == RETRO_DEVICE_JOYPAD)
    {
        if (core_input_supports_bitmasks)
            res = emu_mapping_keys[port] & 0xFFFF;
        else
            res = emu_mapping_keys[port] & RETRO_BITMASK_KEY(id);
    }

    return res;
}
