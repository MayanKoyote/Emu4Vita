#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <psp2/kernel/threadmgr/thread.h>

#include "activity/activity.h"
#include "list/cheat_list.h"
#include "setting/setting.h"
#include "emu/emu.h"
#include "config.h"
#include "utils.h"

enum cheat_handler_type
{
    CHEAT_HANDLER_TYPE_EMU = 0,
    CHEAT_HANDLER_TYPE_RETRO,
    CHEAT_HANDLER_TYPE_END
};

enum cheat_rumble_type
{
    RUMBLE_TYPE_DISABLED = 0,
    RUMBLE_TYPE_CHANGES,
    RUMBLE_TYPE_DOES_NOT_CHANGE,
    RUMBLE_TYPE_INCREASE,
    RUMBLE_TYPE_DECREASE,
    RUMBLE_TYPE_EQ_VALUE,
    RUMBLE_TYPE_NEQ_VALUE,
    RUMBLE_TYPE_LT_VALUE,
    RUMBLE_TYPE_GT_VALUE,
    RUMBLE_TYPE_INCREASE_BY_VALUE,
    RUMBLE_TYPE_DECREASE_BY_VALUE,
    RUMBLE_TYPE_END_LIST
};

static SceUID cheat_thid = -1;
static int cheat_run = 0;
static int cheat_pause = 1;
static int cheat_reset = 0;
static int wait_5_second = 1;
static uint8_t *memory_data = NULL;
static size_t memory_size = 0;

static int makeCheatPath(char *path)
{
    char name[MAX_NAME_LENGTH];
    MakeCurrentFileName(name);
    char base_name[MAX_NAME_LENGTH];
    MakeBaseName(base_name, name, MAX_NAME_LENGTH);
    snprintf(path, MAX_PATH_LENGTH, "%s/%s.cht", (CORE_CHEATS_DIR), base_name);
    return 0;
}

static int makeCheatPath2(char *path)
{
    MakeCurrentFilePath(path);
    char *p = strrchr(path, '.');
    if (!p++)
        p = path + strlen(path);
    strcpy(p, "cht");
    return 0;
}

void Emu_PauseCheat()
{
    cheat_pause = 1;
}

void Emu_ResumeCheat()
{
    cheat_pause = 0;
}

void Emu_CleanCheatOption()
{
    Setting_SetCheatMenu(NULL);
    if (core_cheat_list)
        LinkedListDestroy(core_cheat_list);
    core_cheat_list = NULL;
}

int Emu_UpdateCheatOption()
{
    cheat_reset = 1;

    return 0;
}

int Emu_ResetCheatOption()
{
    return CheatListResetConfig(core_cheat_list);
}

int Emu_LoadCheatOption()
{
    Emu_CleanCheatOption();

    core_cheat_list = NewCheatList();
    if (!core_cheat_list)
        goto FAILED;

    char path[1024];
    makeCheatPath2(path);
    if (CheatListGetEntries(core_cheat_list, path) < 0)
    {
        AppLog("[CHEAT] CheatListGetEntries failed: %s\n", path);
        makeCheatPath(path);
        CheatListGetEntries(core_cheat_list, path);
    }
    if (LinkedListGetLength(core_cheat_list) <= 0)
    {
        AppLog("[CHEAT] CheatListGetEntries failed: %s\n", path);
        goto FAILED;
    }
    AppLog("[CHEAT] CheatListGetEntries OK: %s\n", path);

    MakeConfigPath(path, CHEAT_CONFIG_NAME, TYPE_CONFIG_GAME);
    CheatListLoadConfig(core_cheat_list, path);

    Setting_SetCheatMenu(core_cheat_list);
    AppLog("[CHEAT] Emu_LoadCheatOption OK!\n");
    return 0;

FAILED:
    AppLog("[CHEAT] Emu_LoadCheatOption failed!\n");
    return -1;
}

int Emu_SaveCheatOption()
{
    if (!core_cheat_list)
        goto FAILED;

    char path[1024];
    MakeConfigPath(path, CHEAT_CONFIG_NAME, TYPE_CONFIG_GAME);
    if (CheatListSaveConfig(core_cheat_list, path) < 0)
        goto FAILED;

    AppLog("[CHEAT] Emu_SaveCheatOption OK!\n");
    return 0;

FAILED:
    AppLog("[CHEAT] Emu_SaveCheatOption failed!\n");
    return -1;
}

static void GetMemory()
{
    memory_data = retro_get_memory_data(RETRO_MEMORY_SYSTEM_RAM);
    memory_size = retro_get_memory_size(RETRO_MEMORY_SYSTEM_RAM);
    AppLog("[CHEAT] GetMemory %08x %08x\n", memory_data, memory_size);
}

static int SetupRetroCheatMeta(int bitsize, uint32_t *bytes_per_item, uint32_t *bits, uint32_t *mask)
{
    switch (bitsize)
    {
    case 0:
        *bytes_per_item = 1;
        *bits = 1;
        *mask = 1;
        break;

    case 1:
        *bytes_per_item = 1;
        *bits = 2;
        *mask = 2;
        break;

    case 2:
        *bytes_per_item = 1;
        *bits = 4;
        *mask = 0xf;
        break;

    case 3:
        *bytes_per_item = 1;
        *bits = 8;
        *mask = 0xff;
        break;

    case 4:
        *bytes_per_item = 2;
        *bits = 8;
        *mask = 0xffff;
        break;

    case 5:
        *bytes_per_item = 4;
        *bits = 8;
        *mask = 0xffffffff;
        break;

    default:
        // AppLog("wrong value of memory_search_size: %d", bitsize);
        return 0;
    }

    return 1;
}

static uint32_t GetCurrentValue(int address, int bytes_per_item, int big_endian)
{
    uint8_t *curr = memory_data + address;
    uint32_t value;

    switch (bytes_per_item)
    {
    case 2:
        value = big_endian ? (curr[0] << 8) | curr[1] : curr[0] | (curr[1] << 8);
        break;
    case 4:
        value = big_endian ? (curr[0] << 24) | (curr[1] << 16) | (curr[2] << 8) | curr[3] : curr[0] | (curr[1] << 8) | (curr[2] << 16) | curr[3] << 24;
        break;
    case 1:
    default:
        value = *curr;
        break;
    }

    return value;
}

static void SetCurrentValue(int address, int bytes_per_item, int bits, int big_endian, uint32_t address_mask, int value)
{
    uint8_t *curr = memory_data + address;
    // AppLog("SetCurrentValue %08x %08x %08x %x %d\n", memory_data, address, curr, value, bytes_per_item);

    switch (bytes_per_item)
    {
    case 2:
        if (big_endian)
        {
            curr[0] = (value >> 8) & 0xff;
            curr[1] = value & 0xff;
        }
        else
        {
            curr[0] = value & 0xff;
            curr[1] = (value >> 8) & 0xff;
        }
        break;

    case 4:
        if (big_endian)
        {
            curr[0] = (value >> 24) & 0xff;
            curr[1] = (value >> 16) & 0xff;
            curr[2] = (value >> 8) & 0xff;
            curr[3] = value & 0xff;
        }
        else
        {
            curr[0] = value & 0xff;
            curr[1] = (value >> 8) & 0xff;
            curr[2] = (value >> 16) & 0xff;
            curr[3] = (value >> 24) & 0xff;
        }
        break;

    case 1:
        if (bits < 8)
        {
            uint8_t mask;
            uint8_t v = curr[0];
            for (int i = 0; i < 8; i++)
            {
                if ((address_mask >> i) & 1)
                {
                    mask = ~((1 << i) & 0xff);
                    v &= mask;
                    v |= ((value >> i) & 1) << i;
                }
            }
            curr[0] = v;
        }
        else
            curr[0] = value & 0xff;
        break;

    default:
        curr[0] = value & 0xff;
        break;
    }
}

static void ApplyRetroCheat(const CheatListEntryData *data, int *run_cheat)
{
    uint32_t bytes_per_item;
    uint32_t bits;
    uint32_t value;
    uint32_t mask;
    int set_value = 0;

    if (!(*run_cheat))
    {
        *run_cheat = 1;
        return;
    }

    if (!memory_data)
        GetMemory();

    if (!(memory_data && memory_size && SetupRetroCheatMeta(data->memory_search_size, &bytes_per_item, &bits, &mask)))
        return;

    value = GetCurrentValue(data->address, bytes_per_item, data->big_endian);

    switch (data->cheat_type)
    {
    case CHEAT_TYPE_SET_TO_VALUE:
        value = data->value;
        set_value = 1;
        break;

    case CHEAT_TYPE_INCREASE_VALUE:
        value += data->value;
        set_value = 1;
        break;

    case CHEAT_TYPE_DECREASE_VALUE:
        value -= data->value;
        set_value = 1;
        break;

    case CHEAT_TYPE_RUN_NEXT_IF_EQ:
        *run_cheat = (data->value == value);
        break;

    case CHEAT_TYPE_RUN_NEXT_IF_NEQ:
        *run_cheat = (data->value != value);
        break;

    case CHEAT_TYPE_RUN_NEXT_IF_LT:
        *run_cheat = (data->value < value);
        break;

    case CHEAT_TYPE_RUN_NEXT_IF_GT:
        *run_cheat = (data->value > value);
        break;

    default:
        // AppLog("warning: wrong cheat type: %d\n", data->cheat_type);
        break;
    }

    if (set_value)
    {
        int address = data->address;
        for (int i = 0; i < data->repeat_count; i++)
        {
            SetCurrentValue(address, bytes_per_item, bits, data->big_endian, data->address_bit_position, value);
            value += data->repeat_add_to_value;
            value %= mask;

            if (bits < 8)
            {
                int address_mask = data->address_bit_position;
                for (int i = 0; i < data->repeat_add_to_address; i++)
                {
                    address_mask = (address_mask << mask) & 0xff;
                    if (address_mask == 0)
                    {
                        address_mask = mask;
                        address++;
                    }
                }
            }
            else
                address += data->repeat_add_to_address * bytes_per_item;

            address %= memory_size;
        }
    }
}

static int ApplyCheatOption()
{
    if (cheat_reset)
    {
        retro_cheat_reset();
        cheat_reset = 0;
    }

    if (!core_cheat_list || LinkedListGetLength(core_cheat_list) <= 0)
        return -1;

    LinkedListEntry *entry = LinkedListHead(core_cheat_list);
    int index = 0;
    int run_cheat = 1;

    while (entry)
    {
        CheatListEntryData *data = (CheatListEntryData *)LinkedListGetEntryData(entry);
        if (data->enable)
        {
            if (data->code && *data->code)
            {
                // printf("[CHEAT] ApplyCheatOption: %s = %s\n", data->desc, data->code);
                retro_cheat_set(index, 1, data->code);
            }
            else if (data->handler == CHEAT_HANDLER_TYPE_RETRO)
            {
                // AppLog("[CHEAT] ApplyRetroCheat\n");
                ApplyRetroCheat(data, &run_cheat);
            }
        }

        entry = LinkedListNext(entry);
        index++;
    }

    return 0;
}

static int ApplyCheatOptionThreadFunc(SceSize args, void *argp)
{
    AppLog("[CHEAT] Cheat thread start.\n");

    if (wait_5_second)
        sceKernelDelayThread(5000000);

    while (cheat_run)
    {
        if (cheat_pause || !core_cheat_list || LinkedListGetLength(core_cheat_list) <= 0 || !Emu_IsGameRunning())
        {
            sceKernelDelayThread(1000);
            continue;
        }

        ApplyCheatOption();
        sceKernelDelayThread(1000);
    }

    AppLog("[CHEAT] Cheat thread exit.\n");
    sceKernelExitThread(0);
    return 0;
}

static int ExitApplyCheatOptiontThread()
{
    if (cheat_thid >= 0)
    {
        cheat_run = 0;
        sceKernelWaitThreadEnd(cheat_thid, NULL, NULL);
        sceKernelDeleteThread(cheat_thid);
        cheat_thid = -1;
    }

    return 0;
}

static int StartApplyCheatOptionThread()
{
    int ret = -1;

    if (cheat_thid >= 0)
        ExitApplyCheatOptiontThread();

    ret = cheat_thid = sceKernelCreateThread("cheat_thread", ApplyCheatOptionThreadFunc, 0x10000100, 0x10000, 0, 0, NULL);
    if (cheat_thid >= 0)
    {
        cheat_run = 1;
        ret = sceKernelStartThread(cheat_thid, 0, NULL);
    }

    return ret;
}

int Emu_InitCheat(int wait5sec)
{
    retro_cheat_reset();
    cheat_reset = 0;
    cheat_pause = 1;
    memory_data = NULL;
    memory_size = 0;
    wait_5_second = wait5sec;

    if (Emu_LoadCheatOption() < 0)
        return -1;

    return StartApplyCheatOptionThread();
}

int Emu_DeinitCheat()
{
    ExitApplyCheatOptiontThread();
    Emu_CleanCheatOption();

    return 0;
}
