#ifndef __M_CHEAT_LIST_H__
#define __M_CHEAT_LIST_H__

#include "linked_list.h"

typedef struct CheatListEntryData
{
    char *desc;                    // = "无限血量"
    uint32_t enable;               // = "true"
    char *code;                    // = ""
    int address;                   // = "20793"
    int address_bit_position;      // = "255"
    int big_endian;                // = "false"
    int cheat_type;                // = "1"
    int handler;                   // = "1"
    int memory_search_size;        // = "3"
    int repeat_add_to_address;     // = "1"
    int repeat_add_to_value;       // = "0"
    int repeat_count;              // = "1"
    int rumble_port;               // = "0"
    int rumble_primary_duration;   // = "0"
    int rumble_primary_strength;   // = "0"
    int rumble_secondary_duration; // = "0"
    int rumble_secondary_strength; // = "0"
    int rumble_type;               // = "0"
    int rumble_value;              // = "0"
    int value;                     // = "38"
} CheatListEntryData;

int CheatListGetEntriesFromBuffer(LinkedList *list, void *buffer, int size);
int CheatListGetEntries(LinkedList *list, const char *path);
LinkedList *NewCheatList();

int CheatListResetConfig(LinkedList *list);
int CheatListLoadConfig(LinkedList *list, const char *path);
int CheatListSaveConfig(LinkedList *list, const char *path);

#endif