#ifndef __M_EMU_REWIND_H__
#define __M_EMU_REWIND_H__
#include <stdint.h>
#include <stddef.h>

// 参考 RetroArch 的 state_manager.h/c

void Emu_InitRewind();
void Emu_DeinitRewind();
void Emu_StartRewindGame();
void Emu_StopRewindGame();
int Emu_IsInRewinding();

#endif