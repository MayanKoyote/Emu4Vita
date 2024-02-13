#include <stdlib.h>
#include <string.h>
#include <libretro.h>
#include <psp2/kernel/processmgr.h>
#include "emu_rewind.h"
#include "emu_audio.h"
#include "utils.h"

// 每 0.1 秒记录一次
#define NEXT_STATE_PERIOD 100000

typedef struct
{
    /* Rewind support. */
    uint8_t *data;      // 分配得到的内存地址
    uint8_t *current;   // 指向当前的状态
    uint8_t *header;    // 指向最早的状态
    uint8_t *tail;      // 指向 data 的尾部
    size_t block_size;  // 每个块的大小, 0x10 向上对齐
    size_t size;        // 每个块的实际大小
    uint64_t next_time; // 下一次记录的时间
} RewindState;

RewindState rewind_state = {0};
int rewind_key_pressed = 0;

void Emu_InitRewind(size_t buffer_size)
{
    Emu_DeinitRewind();

    rewind_state.size = retro_serialize_size();
    rewind_state.block_size = (rewind_state.size + 0xf) & -0x10;
    buffer_size = (buffer_size / rewind_state.block_size) * rewind_state.block_size;
    rewind_state.data = malloc(buffer_size);
    rewind_state.header = rewind_state.data;
    rewind_state.current = rewind_state.data;
    rewind_state.tail = rewind_state.data + buffer_size;

    AppLog("[REWIND] Rewind_Init: buf size: %08x, frames: %d\n", buffer_size, buffer_size / rewind_state.block_size);
}

void Emu_DeinitRewind()
{
    if (rewind_state.data)
    {
        free(rewind_state.data);
    }
    memset(&rewind_state, 0, sizeof(rewind_state));
}

void Emu_RewindCheck()
{
    if (rewind_state.data == NULL)
    {
        return;
    }

    if (rewind_key_pressed)
    {
        if (rewind_state.current != rewind_state.header)
        {
            retro_unserialize(rewind_state.current, rewind_state.size);
            rewind_state.current -= rewind_state.block_size;
            if (rewind_state.current < rewind_state.data)
            {
                rewind_state.current = rewind_state.tail - rewind_state.block_size;
            }
            Emu_CleanAudioSound();
        }
        rewind_key_pressed = 0;
    }
    else if (sceKernelGetProcessTimeWide() >= rewind_state.next_time)
    {
        // AppLog("[REWIND] %08x %08x\n", rewind_state.current, rewind_state.header);
        rewind_state.current += rewind_state.block_size;
        if (rewind_state.current >= rewind_state.tail)
            rewind_state.current = rewind_state.data;
        if (rewind_state.current == rewind_state.header)
        {
            rewind_state.header += rewind_state.block_size;
            if (rewind_state.header >= rewind_state.tail)
                rewind_state.header = rewind_state.data;
        }

        retro_serialize(rewind_state.current, rewind_state.block_size);

        rewind_state.next_time = sceKernelGetProcessTimeWide() + NEXT_STATE_PERIOD;
        // AppLog("[REWIND] %lld\n", rewind_state.next_time);
    }
}