#ifndef __M_EMU_REWIND_H__
#define __M_EMU_REWIND_H__
#include <stdint.h>
#include <stddef.h>

// 参考 RetroArch 的 state_manager.h/c

extern int rewind_key_pressed;
extern int in_rewinding;

void Emu_InitRewind(size_t buffer_size);
void Emu_DeinitRewind();
void Emu_RewindCheck();

#endif