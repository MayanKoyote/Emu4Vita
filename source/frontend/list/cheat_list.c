#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "cheat_list.h"
#include "linked_list.h"
#include "config_list.h"
#include "utils_string.h"
#include "file.h"

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

    // printf("checkCheatConfig: idx: %d, idx2: %d\n", idx, idx2);

    if (idx2 != idx)
        return 0; // 合法但非当前index

    p2++; // 前进跳过'_'

    // int found = 1;

    if (strcmp(p2, "desc") == 0)
    {
        if (!data->desc)
        {
            data->desc = malloc(strlen(value) + 1);
            if (data->desc)
                strcpy(data->desc, value);
        }
    }
    else if (strcmp(p2, "code") == 0)
    {
        if (!data->code)
        {
            data->code = malloc(strlen(value) + 1);
            if (data->code)
                strcpy(data->code, value);
        }
    }
    else if (strcmp(p2, "enable") == 0)
    {
        data->enable = StringToBoolean(value);
    }
    else if (strcmp(p2, "address") == 0)
    {
        data->address = StringToDecimal(value);
    }
    else if (strcmp(p2, "address_bit_position") == 0)
    {
        data->address = StringToDecimal(value);
    }
    else if (strcmp(p2, "big_endian") == 0)
    {
        data->address = StringToBoolean(value);
    }
    else if (strcmp(p2, "cheat_type") == 0)
    {
        data->address = StringToDecimal(value);
    }
    else if (strcmp(p2, "handler") == 0)
    {
        data->address = StringToDecimal(value);
    }
    else if (strcmp(p2, "memory_search_size") == 0)
    {
        data->address = StringToDecimal(value);
    }
    else if (strncmp(p2, "repeat_", 7) == 0)
    {
        p2 += 7;
        if (strcmp(p2, "add_to_address") == 0)
        {
            data->address = StringToDecimal(value);
        }
        else if (strcmp(p2, "add_to_value") == 0)
        {
            data->address = StringToDecimal(value);
        }
        else if (strcmp(p2, "count") == 0)
        {
            data->address = StringToDecimal(value);
        }
    }
    else if (strncmp(p2, "rumble_", 7) == 0)
    {
        p2 += 7;
        if (strcmp(p2, "port") == 0)
        {
            data->address = StringToDecimal(value);
        }
        else if (strcmp(p2, "primary_duration") == 0)
        {
            data->address = StringToDecimal(value);
        }
        else if (strcmp(p2, "primary_strength") == 0)
        {
            data->address = StringToDecimal(value);
        }
        else if (strcmp(p2, "secondary_duration") == 0)
        {
            data->address = StringToDecimal(value);
        }
        else if (strcmp(p2, "secondary_strength") == 0)
        {
            data->address = StringToDecimal(value);
        }
        else if (strcmp(p2, "type") == 0)
        {
            data->address = StringToDecimal(value);
        }
        else if (strcmp(p2, "value") == 0)
        {
            data->address = StringToDecimal(value);
        }
    }
    else if (strcmp(p2, "value") == 0)
    {
        data->address = StringToDecimal(value);
    }
    else
    {
        // found = 0;
    }

    // if (found)
    //     printf("Add Found: index: %d, key: %s, value: %s\n", idx, p2, value);

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
    int size = AllocateReadFile(path, &buffer);
    if (size < 0)
        return size;

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
