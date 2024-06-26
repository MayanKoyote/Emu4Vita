#ifndef __M_EMU_H__
#define __M_EMU_H__

#include "emu_retro.h"
#include "emu_game.h"
#include "emu_audio.h"
#include "emu_video.h"
#include "emu_input.h"
#include "emu_save.h"
#include "emu_state.h"
#include "emu_disk.h"
#include "emu_cheat.h"
#include "emu_rewind.h"

#if defined(WANT_ARCHIVE_ROM)
#include "emu_archive.h"
#endif

int Emu_Init();
void Emu_Deinit();

#endif