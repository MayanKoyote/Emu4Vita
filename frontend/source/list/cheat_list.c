#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <psp2/io/fcntl.h>

#include "cheat_list.h"
#include "linked_list.h"
#include "config_list.h"
#include "utils_string.h"
#include "file.h"
#include "config.h"

static void freeEntryData(void *data)
{
    CheatListEntryData *e_data = (CheatListEntryData *)data;
    if (e_data)
    {
        if (e_data->desc)
            free(e_data->desc);
        if (e_data->code)
            free(e_data->code);
        free(e_data);
    }
}

static int checkCheatConfig(CheatListEntryData *data, int idx, char *key, char *value)
{
    // 示例 key: cheat10_desc

    if (!data || !key || !value)
        return -1;

    // printf("checkCheatConfig: key: %s, value: %s\n", key, value);

    // 判断起始字符串
    if (strncmp(key, "cheat", 5) != 0)
        return -1; // 非法key

    // 判断index
    char *p = key + 5;
    char *p2 = p;
    while (*p2 >= '0' && *p2 <= '9')
        p2++;

    if (p2 == p || *p2 != '_') // 无index或后面不跟'_'
        return -1;             // 非法key

    char ch = *p2;
    *p2 = '\0';
    int idx2 = StringToDecimal(p); // 字符串转整数index
    *p2 = ch;

    if (idx2 != idx)
        return 0; // 合法但非当前index

    p = ++p2; // 前进跳过'_'

    int found = 1;

    if (strcmp(p, "desc") == 0)
    {
        if (!data->desc)
        {
            data->desc = malloc(strlen(value) + 1);
            if (data->desc)
                strcpy(data->desc, value);
        }
    }
    else if (strcmp(p, "code") == 0)
    {
        if (!data->code)
        {
            int value_len = strlen(value);
            if (value_len > 0)
            {
                data->code = malloc(value_len + 1);
                if (data->code)
                    strcpy(data->code, value);
            }
        }
    }
    // else if (strcmp(p, "enable") == 0)
    // {
    //    data->enable = StringToBoolean(value);
    // }
    else if (strcmp(p, "address") == 0)
    {
        data->address = StringToDecimal(value);
    }
    else if (strcmp(p, "address_bit_position") == 0)
    {
        data->address_bit_position = StringToDecimal(value);
    }
    else if (strcmp(p, "big_endian") == 0)
    {
        data->big_endian = StringToBoolean(value);
    }
    else if (strcmp(p, "cheat_type") == 0)
    {
        data->cheat_type = StringToDecimal(value);
    }
    else if (strcmp(p, "handler") == 0)
    {
        data->handler = StringToDecimal(value);
    }
    else if (strcmp(p, "memory_search_size") == 0)
    {
        data->memory_search_size = StringToDecimal(value);
    }
    else if (strncmp(p, "repeat_", 7) == 0)
    {
        p2 = p + 7;
        if (strcmp(p2, "add_to_address") == 0)
        {
            data->repeat_add_to_address = StringToDecimal(value);
        }
        else if (strcmp(p2, "add_to_value") == 0)
        {
            data->repeat_add_to_value = StringToDecimal(value);
        }
        else if (strcmp(p2, "count") == 0)
        {
            data->repeat_count = StringToDecimal(value);
        }
    }
    else if (strncmp(p, "rumble_", 7) == 0)
    {
        p2 = p + 7;
        if (strcmp(p2, "port") == 0)
        {
            data->rumble_port = StringToDecimal(value);
        }
        else if (strcmp(p2, "primary_duration") == 0)
        {
            data->rumble_primary_duration = StringToDecimal(value);
        }
        else if (strcmp(p2, "primary_strength") == 0)
        {
            data->rumble_primary_strength = StringToDecimal(value);
        }
        else if (strcmp(p2, "secondary_duration") == 0)
        {
            data->rumble_secondary_duration = StringToDecimal(value);
        }
        else if (strcmp(p2, "secondary_strength") == 0)
        {
            data->rumble_secondary_strength = StringToDecimal(value);
        }
        else if (strcmp(p2, "type") == 0)
        {
            data->rumble_type = StringToDecimal(value);
        }
        else if (strcmp(p2, "value") == 0)
        {
            data->rumble_value = StringToDecimal(value);
        }
    }
    else if (strcmp(p, "value") == 0)
    {
        data->value = StringToDecimal(value);
    }
    else
    {
        found = 0;
    }

    if (found)
    {
        // printf("Add Found: index: %d, key: %s, value: %s\n", idx, p, value);
    }

    return 1;
}

int CheatListGetEntriesFromBuffer(LinkedList *list, void *buffer, int size)
{
    if (!list)
        return -1;

    LinkedList *cfg_list = NewConfigList();
    if (!cfg_list)
        return -1;

    ConfigListGetEntriesFromBuffer(cfg_list, buffer, size);

    LinkedListEntry *cht_entry = NULL;
    CheatListEntryData *cht_data = NULL;
    int i;
    for (i = 0; LinkedListGetLength(cfg_list) > 0; i++)
    {
        cht_data = (CheatListEntryData *)calloc(1, sizeof(CheatListEntryData));
        if (!cht_data)
            break;

        cht_data->desc = NULL;
        cht_data->code = NULL;
        cht_data->big_endian = 0;
        cht_data->repeat_count = 1;
        cht_data->cheat_type = CHEAT_TYPE_SET_TO_VALUE;
        cht_data->memory_search_size = 3;
        cht_data->repeat_add_to_value = 0;
        cht_data->repeat_add_to_address = 1;
        cht_data->address_bit_position = 0xff;

        cht_entry = LinkedListAdd(list, cht_data);

        LinkedListEntry *cfg_entry = LinkedListHead(cfg_list);

        while (cfg_entry)
        {
            LinkedListEntry *next_cfg_entry = LinkedListNext(cfg_entry);
            ConfigListEntryData *cfg_data = (ConfigListEntryData *)LinkedListGetEntryData(cfg_entry);

            int ret = checkCheatConfig(cht_data, i, cfg_data->key, cfg_data->value);
            if (ret == -1 || ret == 1)
                LinkedListRemove(cfg_list, cfg_entry);

            cfg_entry = next_cfg_entry;
        }
    }

    cht_data = (CheatListEntryData *)LinkedListGetEntryData(cht_entry);
    if (cht_data && !cht_data->desc)
        LinkedListRemove(list, cht_entry);

    // cht_entry = LinkedListHead(list);
    // i = 0;
    // while (cht_entry)
    // {
    //    cht_data = (CheatListEntryData *)LinkedListGetEntryData(cht_entry);
    //    printf("cheat_list: index: %d, desc: %s, enable: %d\n", i, cht_data->desc, cht_data->enable);
    //    cht_entry = LinkedListNext(cht_entry);
    //    i++;
    //}

    LinkedListDestroy(cfg_list);

    return 0;
}

int CheatListGetEntries(LinkedList *list, const char *path)
{
    if (!list)
        return -1;

    void *buffer = NULL;
    size_t size = 0;
    if (AllocateReadFile(path, &buffer, &size) < 0)
        return -1;

    CheatListGetEntriesFromBuffer(list, buffer, size);

    free(buffer);

    return 0;
}

LinkedList *NewCheatList()
{
    LinkedList *list = NewLinkedList();
    if (!list)
        return NULL;

    LinkedListSetFreeEntryDataCallback(list, freeEntryData);
    return list;
}

int CheatListResetConfig(LinkedList *list)
{
    if (!list)
        return -1;

    LinkedListEntry *entry = LinkedListHead(list);

    while (entry)
    {
        CheatListEntryData *data = (CheatListEntryData *)LinkedListGetEntryData(entry);
        data->enable = 0;
        entry = LinkedListNext(entry);
    }

    return 0;
}

int CheatListLoadConfig(LinkedList *list, const char *path)
{
    if (!list)
        return -1;

    LinkedList *config_list = NewConfigList();
    if (config_list)
        ConfigListGetEntries(config_list, path);

    LinkedListEntry *entry = LinkedListHead(list);

    while (entry)
    {
        CheatListEntryData *data = (CheatListEntryData *)LinkedListGetEntryData(entry);
        data->enable = 0;

        if (data->desc && config_list)
        {
            LinkedListEntry *find = ConfigListFindEntryByKey(config_list, data->desc);
            if (find)
            {
                ConfigListEntryData *c_data = (ConfigListEntryData *)LinkedListGetEntryData(find);
                data->enable = StringToDecimal(c_data->value);
                LinkedListRemove(config_list, find);
            }
        }

        entry = LinkedListNext(entry);
    }

    if (config_list)
        LinkedListDestroy(config_list);

    return 0;
}

int CheatListSaveConfig(LinkedList *list, const char *path)
{
    if (!list)
        return -1;

    SceUID fd = sceIoOpen(path, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
    if (fd < 0)
        return fd;

    int ret = 0;
    char string[MAX_CONFIG_LINE_LENGTH];
    LinkedListEntry *entry = LinkedListHead(list);

    while (entry)
    {
        CheatListEntryData *data = (CheatListEntryData *)LinkedListGetEntryData(entry);
        snprintf(string, MAX_CONFIG_LINE_LENGTH, "%s = \"%d\"\n", data->desc, data->enable);

        if ((ret = sceIoWrite(fd, string, strlen(string))) < 0)
            break;

        entry = LinkedListNext(entry);
    }

    sceIoClose(fd);
    if (ret < 0)
        sceIoRemove(path);
    return ret;
}
