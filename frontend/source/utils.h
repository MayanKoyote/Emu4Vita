#ifndef __M_UTILS_H__
#define __M_UTILS_H__

#include <stdint.h>

#include <psp2/rtc.h>
#include <psp2/ctrl.h>

enum TypeMove
{
    TYPE_MOVE_NONE,
    TYPE_MOVE_UP,
    TYPE_MOVE_DOWN,
    TYPE_MOVE_LEFT,
    TYPE_MOVE_RIGHT,
};

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#define lerp(value, from_max, to_max) ((((value * 10) * (to_max * 10)) / (from_max * 10)) / 10)

int AppLog(const char *text, ...);

int HasEndSlash(const char *path);
int RemoveEndSlash(char *path);
int AddEndSlash(char *path);

void ConvertUtcToLocalTime(SceDateTime *time_local, SceDateTime *time_utc);
void ConvertLocalTimeToUtc(SceDateTime *time_utc, SceDateTime *time_local);

void GetSizeString(char string[16], uint64_t size);
void GetDateString(char string[24], int date_format, SceDateTime *time);
void GetTimeString(char string[16], int time_format, SceDateTime *time);
void GetDurationString(char string[16], uint64_t ms);

void RefreshListPos(int *top_pos, int *focus_pos, int length, int lines);
void RefreshListPosNoFocus(int *top_pos, int length, int lines);

void MoveListPos(int type, int *top_pos, int *focus_pos, int length, int lines);
void MoveListPosNoFocus(int type, int *top_pos, int length, int lines);

void LockHomeKey();
void UnlockHomeKey();
void LockUsbConnection();
void UnlockUsbConnection();
void LockQuickMenu();
void UnlockQuickMenu();
void AutoUnlockQuickMenu();

void LockSuspend();
void UnlockSuspend();
void LockOledOff();
void UnlockOledOff();
void LockOledDimming();
void UnlockOledDimming();

void InitPowerTickThread();

void SetControlEventEnabled(int enable);
int IsControlEventEnabled();
void SetPSbuttonEventEnabled(int enable);
int IsPSbuttonEventEnabled();

uint64_t GetTotalHeadMemSize();
uint64_t GetFreeHeadMemSize();
uint64_t GetUsedHeadMemSize();

int GetUTF8Count(const char *utf8);

#endif
