#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "emu.h"

int Emu_Init()
{
    Emu_InitRetro();
    return 0;
}

void Emu_Deinit()
{
    Emu_FinishGame();
    Emu_DeinitRetro();
}
