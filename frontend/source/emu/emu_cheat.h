#ifndef __M_EMU_CHEAT_H__
#define __M_EMU_CHEAT_H__

void Emu_PauseCheat();
void Emu_ResumeCheat();

void Emu_CleanCheatOption();
int Emu_UpdateCheatOption();
int Emu_ResetCheatOption();
int Emu_LoadCheatOption();
int Emu_SaveCheatOption();

int Emu_InitCheat(int wait5sec);
int Emu_DeinitCheat();

#endif