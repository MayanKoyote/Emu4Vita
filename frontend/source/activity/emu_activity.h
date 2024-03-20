#ifndef __M_EMU_ACITVITY_H__
#define __M_EMU_ACITVITY_H__

#include "gui/gui.h"

extern GUI_Activity emu_activity;

void Emu_SetMicrosPerFrame(uint64_t micros);
uint64_t Emu_GetMicrosPerFrame();

#endif
