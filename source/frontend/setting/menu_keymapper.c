#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "emu/emu.h"
#include "setting.h"
#include "lang.h"
#include "config.h"

#ifndef __SETTING_MENU_EMBED
#include "setting_variables.c"
#endif

static CheckBoxOptionMenuItem emu_key_mapper_option_items[] = {
#if defined(WSC_BUILD)
    {{LANG_BUTTON_X1, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_LEFT)},
    {{LANG_BUTTON_X2, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_UP)},
    {{LANG_BUTTON_X3, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_RIGHT)},
    {{LANG_BUTTON_X4, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_DOWN)},
#else
    {{LANG_BUTTON_LEFT, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_LEFT)},
    {{LANG_BUTTON_UP, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_UP)},
    {{LANG_BUTTON_RIGHT, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_RIGHT)},
    {{LANG_BUTTON_DOWN, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_DOWN)},
#endif
#if defined(FC_BUILD) || defined(GBC_BUILD) || defined(NGP_BUILD)
    {{LANG_BUTTON_A, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_A)},
    {{LANG_BUTTON_B, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_B)},
    {{LANG_BUTTON_SELECT, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_SELECT)},
    {{LANG_BUTTON_START, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_START)},
#if defined(FC_BUILD)
    {{LANG_FDS_DISK_SIDE_CHANGE, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_L)},
    {{LANG_FDS_INSERT_EJECT_DISK, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_R)},
#endif
#elif defined(GBA_BUILD)
    {{LANG_BUTTON_A, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_A)},
    {{LANG_BUTTON_B, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_B)},
    {{LANG_BUTTON_L, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_L)},
    {{LANG_BUTTON_R, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_R)},
    {{LANG_BUTTON_SELECT, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_SELECT)},
    {{LANG_BUTTON_START, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_START)},
#elif defined(SFC_BUILD)
    {{LANG_BUTTON_A, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_A)},
    {{LANG_BUTTON_B, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_B)},
    {{LANG_BUTTON_X, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_X)},
    {{LANG_BUTTON_Y, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_Y)},
    {{LANG_BUTTON_L, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_L)},
    {{LANG_BUTTON_R, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_R)},
    {{LANG_BUTTON_SELECT, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_SELECT)},
    {{LANG_BUTTON_START, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_START)},
#elif defined(MD_BUILD)
    {{LANG_BUTTON_A, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_Y)},
    {{LANG_BUTTON_B, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_B)},
    {{LANG_BUTTON_C, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_A)},
    {{LANG_BUTTON_X, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_L)},
    {{LANG_BUTTON_Y, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_X)},
    {{LANG_BUTTON_Z, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_R)},
    {{LANG_SWICTH_MODE, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_SELECT)},
    {{LANG_BUTTON_START, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_START)},
#elif defined(WSC_BUILD)
    {{LANG_BUTTON_Y1, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_L)},
    {{LANG_BUTTON_Y2, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_R2)},
    {{LANG_BUTTON_Y3, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_R)},
    {{LANG_BUTTON_Y4, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_L2)},
    {{LANG_BUTTON_A, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_A)},
    {{LANG_BUTTON_B, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_B)},
    {{LANG_BUTTON_START, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_START)},
#elif defined(PCE_BUILD)
    {{LANG_BUTTON_A, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_A)},
    {{LANG_BUTTON_B, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_B)},
    {{LANG_BUTTON_X, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_X)},
    {{LANG_BUTTON_Y, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_Y)},
    {{LANG_BUTTON_L, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_L)},
    {{LANG_BUTTON_R, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_R)},
    {{LANG_BUTTON_SELECT, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_SELECT)},
    {{LANG_BUTTON_START, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_START)},
    {{LANG_SWICTH_MODE, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_L2)},
#elif defined(FBA_BUILD)
    {{LANG_BUTTON_A, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_B)},
    {{LANG_BUTTON_B, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_A)},
    {{LANG_BUTTON_C, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_Y)},
    {{LANG_BUTTON_D, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_X)},
    {{LANG_BUTTON_E, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_L)},
    {{LANG_BUTTON_F, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_R)},
    {{LANG_BUTTON_G, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_L2)},
    {{LANG_BUTTON_H, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_R2)},
    {{LANG_COIN, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_SELECT)},
    {{LANG_START, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_START)},
#elif defined(PS_BUILD)
    {{LANG_BUTTON_CROSS, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_B)},
    {{LANG_BUTTON_CIRCLE, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_A)},
    {{LANG_BUTTON_SQUARE, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_Y)},
    {{LANG_BUTTON_TRIANGLE, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_X)},
    {{LANG_BUTTON_L, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_L)},
    {{LANG_BUTTON_R, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_R)},
    {{LANG_BUTTON_L2, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_L2)},
    {{LANG_BUTTON_R2, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_R2)},
    {{LANG_BUTTON_L3, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_L3)},
    {{LANG_BUTTON_R3, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_R3)},
    {{LANG_BUTTON_SELECT, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_SELECT)},
    {{LANG_BUTTON_START, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_START)},
#else
    {{LANG_BUTTON_A, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_A)},
    {{LANG_BUTTON_B, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_B)},
    {{LANG_BUTTON_X, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_X)},
    {{LANG_BUTTON_Y, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_Y)},
    {{LANG_BUTTON_L, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_L)},
    {{LANG_BUTTON_R, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_R)},
    {{LANG_BUTTON_L2, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_L2)},
    {{LANG_BUTTON_R2, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_R2)},
    {{LANG_BUTTON_L3, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_L3)},
    {{LANG_BUTTON_R3, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_R3)},
    {{LANG_BUTTON_SELECT, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_SELECT)},
    {{LANG_BUTTON_START, NULL}, 0, (void *)BITMASK_RETRO_KEY(RETRO_DEVICE_ID_JOYPAD_START)},
#endif
};

#define STD_EMU_KEY_MAPPER_OPTION(text_idx, config_element)                                                                   \
    {                                                                                                                         \
        {LANG_NULL, NULL}, emu_key_mapper_option_items, sizeof(emu_key_mapper_option_items) / sizeof(CheckBoxOptionMenuItem), \
            openKeyMapperOptionCallback, NULL, controlKeymapperOptionChangedCallback, &control_config.config_element          \
    }

static CheckBoxOptionMenuItem hot_key_mapper_option_items[] = {
    {{LANG_BUTTON_PSBUTTON, NULL}, 0, (void *)SCE_CTRL_PSBUTTON},
    {{LANG_BUTTON_LEFT, NULL}, 0, (void *)SCE_CTRL_LEFT},
    {{LANG_BUTTON_UP, NULL}, 0, (void *)SCE_CTRL_UP},
    {{LANG_BUTTON_RIGHT, NULL}, 0, (void *)SCE_CTRL_RIGHT},
    {{LANG_BUTTON_DOWN, NULL}, 0, (void *)SCE_CTRL_DOWN},
    {{LANG_BUTTON_CROSS, NULL}, 0, (void *)SCE_CTRL_CROSS},
    {{LANG_BUTTON_CIRCLE, NULL}, 0, (void *)SCE_CTRL_CIRCLE},
    {{LANG_BUTTON_SQUARE, NULL}, 0, (void *)SCE_CTRL_SQUARE},
    {{LANG_BUTTON_TRIANGLE, NULL}, 0, (void *)SCE_CTRL_TRIANGLE},
    {{LANG_BUTTON_L, NULL}, 0, (void *)SCE_CTRL_L1},
    {{LANG_BUTTON_R, NULL}, 0, (void *)SCE_CTRL_R1},
    {{LANG_BUTTON_L2, NULL}, 0, (void *)SCE_CTRL_L2},
    {{LANG_BUTTON_R2, NULL}, 0, (void *)SCE_CTRL_R2},
    {{LANG_BUTTON_L3, NULL}, 0, (void *)SCE_CTRL_L3},
    {{LANG_BUTTON_R3, NULL}, 0, (void *)SCE_CTRL_R3},
    {{LANG_BUTTON_SELECT, NULL}, 0, (void *)SCE_CTRL_SELECT},
    {{LANG_BUTTON_START, NULL}, 0, (void *)SCE_CTRL_START},
    {{LANG_BUTTON_LEFT_ANALOG_LEFT, NULL}, 0, (void *)EXT_CTRL_LEFT_ANLOG_LEFT},
    {{LANG_BUTTON_LEFT_ANALOG_UP, NULL}, 0, (void *)EXT_CTRL_LEFT_ANLOG_UP},
    {{LANG_BUTTON_LEFT_ANALOG_RIGHT, NULL}, 0, (void *)EXT_CTRL_LEFT_ANLOG_RIGHT},
    {{LANG_BUTTON_LEFT_ANALOG_DOWN, NULL}, 0, (void *)EXT_CTRL_LEFT_ANLOG_DOWN},
    {{LANG_BUTTON_RIGHT_ANALOG_LEFT, NULL}, 0, (void *)EXT_CTRL_RIGHT_ANLOG_LEFT},
    {{LANG_BUTTON_RIGHT_ANALOG_UP, NULL}, 0, (void *)EXT_CTRL_RIGHT_ANLOG_UP},
    {{LANG_BUTTON_RIGHT_ANALOG_RIGHT, NULL}, 0, (void *)EXT_CTRL_RIGHT_ANLOG_RIGHT},
    {{LANG_BUTTON_RIGHT_ANALOG_DOWN, NULL}, 0, (void *)EXT_CTRL_RIGHT_ANLOG_DOWN},
};

#define STD_HOT_KEY_MAPPER_OPTION(text_idx, config_element)                                                                   \
    {                                                                                                                         \
        {LANG_NULL, NULL}, hot_key_mapper_option_items, sizeof(hot_key_mapper_option_items) / sizeof(CheckBoxOptionMenuItem), \
            openKeyMapperOptionCallback, NULL, hotkeyOptionUpdateCallback, &hotkey_config.config_element                      \
    }

static void refreshOptionLayout()
{
    option_listview_width = option_itemview_width + OPTION_LISTVIEW_PADDING_L * 2;
    option_listview_dx = menu_listview_dx;
    option_listview_sx = option_listview_dx - option_listview_width;
    option_listview_sy = menu_listview_sy;
    option_listview_dy = menu_listview_dy;
    option_listview_scroll_sx = option_listview_dx;
    option_listview_height = option_listview_dy - option_listview_sy;
    option_listview_n_draw_items = (option_listview_height - OPTION_LISTVIEW_PADDING_T * 2) / option_itemview_height;

    option_scrollbar_track_x = option_listview_dx - GUI_DEF_SCROLLBAR_SIZE - 2;
    option_scrollbar_track_y = option_listview_sy + 2;
    option_scrollbar_track_height = option_listview_height + 4;
}

static void openKeyMapperOptionCallback(CheckBoxOptionMenu *option)
{
    option_itemview_checkbox_width = GUI_GetLineHeight();
    option_itemview_checkbox_height = option_itemview_checkbox_width;
    int name_w = DEFAULT_OPTION_LISTVIEW_WIDTH - OPTION_LISTVIEW_PADDING_L * 2 - OPTION_ITEMVIEW_PADDING_L * 2 - option_itemview_checkbox_width - 8;

    uint32_t config_key = *(uint32_t *)(option->userdata);
    int enabled = config_key & ENABLE_KEY_BITMASK;

    CheckBoxOptionMenuItem *option_item;
    const char *name;
    uint32_t mapping_key;
    int i;
    for (i = 0; i < option->n_items; i++)
    {
        option_item = &(option->items[i]);

        name = GetLangString(&option_item->name);
        if (name)
        {
            int w = GUI_GetTextWidth(name);
            if (w > name_w)
                name_w = w;
        }

        option_item->selected = 0;
        if (!enabled)
            continue;

        mapping_key = (uint32_t)(option_item->userdata);
        if (config_key & mapping_key)
            option_item->selected = 1;
    }

    option_itemview_width = name_w + 8 + option_itemview_checkbox_width + OPTION_ITEMVIEW_PADDING_L * 2;
    option_itemview_height = GUI_GetLineHeight() + OPTION_ITEMVIEW_PADDING_T * 2;
    refreshOptionLayout();
}

static void updateKeyMapperOptionCallback(CheckBoxOptionMenu *option)
{
    if (!option->name.string)
        option->name.string = (char *)malloc(256);
    if (option->name.string)
        option->name.string[0] = '\0';

    uint32_t *config_key = (uint32_t *)(option->userdata);
    int turbo = *config_key & TURBO_KEY_BITMASK;
    uint32_t new_config_key = 0;

    int n_mapped = 0;
    int i;
    for (i = 0; i < option->n_items; i++)
    {
        CheckBoxOptionMenuItem *option_item = &(option->items[i]);

        if (option_item->selected)
        {
            uint32_t mapping_key = (uint32_t)(option_item->userdata);
            new_config_key |= mapping_key;
            if (option->name.string)
            {
                const char *name = GetLangString(&option_item->name);
                if (name)
                {
                    if (n_mapped > 0)
                        strcat(option->name.string, "+");
                    strcat(option->name.string, name);
                }
            }
            n_mapped++;
        }
    }

    if (n_mapped > 0)
    {
        new_config_key |= ENABLE_KEY_BITMASK;
        option->name.lang = LANG_NULL;
    }
    else
    {
        option->name.lang = LANG_DISABLE;
    }

    if (turbo)
        new_config_key |= TURBO_KEY_BITMASK;

    *config_key = new_config_key;
}
