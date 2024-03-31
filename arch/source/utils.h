#ifndef __M_UTILS_H__
#define __M_UTILS_H__

#include <stdint.h>

#include <psp2/rtc.h>
#include <psp2/ctrl.h>
#include <psp2/gxm.h>

#ifdef MIN
#undef MIN
#endif
#ifndef MAX
#undef MAX
#endif
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define ANALOG_CENTER 128
#define ANALOG_THRESHOLD 64
#define ANALOG_SENSITIVITY 16

enum MoveTypes
{
    TYPE_MOVE_NONE,
    TYPE_MOVE_UP,
    TYPE_MOVE_DOWN,
    TYPE_MOVE_LEFT,
    TYPE_MOVE_RIGHT,
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

typedef uint8_t Pad[PAD_N_BUTTONS];

extern Pad old_pad, current_pad, pressed_pad, released_pad, hold_pad, hold2_pad;
extern Pad hold_count, hold2_count;

int APP_LOG(char *text, ...);

void ReadPad();
void CleanPad();

int HasEndSlash(const char *path);
int RemoveEndSlash(char *path);
int AddEndSlash(char *path);

void ConvertUtcToLocalTime(SceDateTime *time_local, SceDateTime *time_utc);
void ConvertLocalTimeToUtc(SceDateTime *time_utc, SceDateTime *time_local);

void GetSizeString(char string[16], uint64_t size);
void GetDateString(char string[24], int date_format, SceDateTime *time);
void GetTimeString(char string[16], int time_format, SceDateTime *time);

void LockHome();
void UnlockHome();
void UnlockHomeEx();
void LockUsbConnection();
void UnlockUsbConnection();
void LockQuickMenu();
void UnlockQuickMenu();

#endif
