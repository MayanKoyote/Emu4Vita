#ifndef __M_EMU_CHEAT_H__
#define __M_EMU_CHEAT_H__

void Emu_PauseCheat();
void Emu_ResumeCheat();

int Emu_CleanCheatOption();
int Emu_UpdateCheatOption();
int Emu_ResetCheatOption();
int Emu_LoadCheatOption();
int Emu_SaveCheatOption();
int Emu_ApplyCheatOption();

int Emu_InitCheat();
int Emu_DeinitCheat();

#endif