#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <psp2/kernel/threadmgr.h>
#include <psp2/io/dirent.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>

#include "activity/activity.h"
#include "emu/emu.h"
#include "gui/gui.h"
#include "setting.h"
#include "setting_window.h"
#include "utils.h"
#include "config.h"
#include "lang.h"
#include "init.h"

#define STATE_LISTVIEW_PADDING_L 0.0f
#define STATE_LISTVIEW_PADDING_T 8.0f

#define STATE_ITEMVIEW_PADDING 8.0f
#define STATE_ITEMVIEW_MARGIN 8.0f

#define OPTION_LINE_SPACE 4.0f

#define STATE_PREVIEW_HEIGHT ((GUI_GetLineHeight() + OPTION_LINE_SPACE) * 4 - OPTION_LINE_SPACE)
#define STATE_PREVIEW_WIDTH(h) (h * 4.0f / 3.0f)

#define STATE_ITEMVIEW_COLOR_DEF_BG COLOR_ALPHA(COLOR_DARKGRAY, 0xAF)
#define STATE_ITEMVIEW_COLOR_FOCUS_BG GUI_DEF_COLOR_FOCUS

#define STATE_LIST_LEN 30
#define N_STATE_COUNTS_PER_LINE 2
#define N_STATE_DRAW_LINES 4

#define STATE_INFO_MARGIN_L 6.0f
#define STATE_INFO_LINE_SPACE 4.0f

#define STATE_PREVIEW_COLOR_BG COLOR_ALPHA(COLOR_GRAY, 0xAF)

typedef struct
{
    int exist;
    GUI_Texture *tex;
    SceOff size;
    SceDateTime time;
} StateListItem;

typedef struct
{
    int name;
    int enable;
} StateMenuItem;

static StateMenuItem state_menu[] = {
    {LANG_STATE_LOAD_STATE, 1},
    {LANG_STATE_SAVE_STATE, 1},
    {LANG_STATE_DELETE_STATE, 1},
    {LANG_CANCEL, 1},
};
#define N_MENU_ITEMS (sizeof(state_menu) / sizeof(StateMenuItem))

StateListItem *state_list = NULL;
static SceUID state_thid = -1;
static int state_thread_stop = 0;

static int list_top_pos = 0, list_focus_pos = 0;
static int option_open = 0;
static int option_focus_pos = 0;

static void refreshStateListPos(int *top_pos, int *focus_pos)
{
    int temp_top_pos = *top_pos;
    int temp_focus_pos = *focus_pos;

    if (temp_focus_pos > STATE_LIST_LEN - 1)
        temp_focus_pos = STATE_LIST_LEN - 1;
    if (temp_focus_pos < 0)
        temp_focus_pos = 0;

    int half_draw_lines = N_STATE_DRAW_LINES / 2;
    int focus_pos_off_line = temp_focus_pos % N_STATE_COUNTS_PER_LINE;
    int list_len_off_line = STATE_LIST_LEN % N_STATE_COUNTS_PER_LINE;

    // Make focus_pos in current line
    temp_top_pos = temp_focus_pos - focus_pos_off_line - (half_draw_lines * N_STATE_COUNTS_PER_LINE);

    int max_top_pos = STATE_LIST_LEN - list_len_off_line - (N_STATE_DRAW_LINES - (list_len_off_line ? 1 : 0)) * N_STATE_COUNTS_PER_LINE;
    if (temp_top_pos > max_top_pos)
        temp_top_pos = max_top_pos;
    if (temp_top_pos < 0)
        temp_top_pos = 0;

    *top_pos = temp_top_pos;
    *focus_pos = temp_focus_pos;
}

static void moveStateListPos(int type)
{
    int temp_top_pos = list_top_pos;
    int temp_focus_pos = list_focus_pos;

    if ((type == TYPE_MOVE_UP) && (temp_focus_pos >= N_STATE_COUNTS_PER_LINE))
        temp_focus_pos -= N_STATE_COUNTS_PER_LINE;
    else if ((type == TYPE_MOVE_DOWN) && (temp_focus_pos < STATE_LIST_LEN - N_STATE_COUNTS_PER_LINE))
        temp_focus_pos += N_STATE_COUNTS_PER_LINE;
    else if (type == TYPE_MOVE_LEFT)
        temp_focus_pos--;
    else if (type == TYPE_MOVE_RIGHT)
        temp_focus_pos++;

    refreshStateListPos(&temp_top_pos, &temp_focus_pos);
    list_top_pos = temp_top_pos;
    list_focus_pos = temp_focus_pos;
}

static void cleanStateItem(int num)
{
    if (!state_list || num < 0 || num >= STATE_LIST_LEN)
        return;

    if (state_list[num].tex)
    {
        GUI_DestroyTexture(state_list[num].tex);
        state_list[num].tex = NULL;
    }
    state_list[num].exist = 0;
}

static void freeStateList()
{
    if (!state_list)
        return;

    int i;
    for (i = 0; i < STATE_LIST_LEN; i++)
    {
        if (state_list[i].tex)
            GUI_DestroyTexture(state_list[i].tex);
    }
    free(state_list);
    state_list = NULL;
}

static void refreshStateItem(int num)
{
    char path[MAX_PATH_LENGTH];

    if (!state_list || num < 0 || num >= STATE_LIST_LEN)
        return;

    MakeSavestatePath(path, num);
    cleanStateItem(num);

    SceUID fd = sceIoOpen(path, SCE_O_RDONLY, 0);
    if (fd < 0)
        return;

    // Read header
    EmuStateHeader header;
    memset(&header, 0, sizeof(EmuStateHeader));
    sceIoRead(fd, &header, sizeof(EmuStateHeader));

    if (header.version != STATES_VERSION)
    {
        sceIoClose(fd);
        return;
    }

    // Read framebuffer
    state_list[num].tex = GUI_CreateTextureFormat(header.preview_width, header.preview_height, GUI_PIXEL_FORMAT_U8U8U8U8_ABGR);
    if (state_list[num].tex)
    {
        sceIoLseek(fd, header.preview_offset, SCE_SEEK_SET);
        sceIoRead(fd, GUI_GetTextureDatap(state_list[num].tex), header.preview_size);
    }

    // Set file stat
    SceIoStat stat;
    memset(&stat, 0, sizeof(SceIoStat));
    if (sceIoGetstat(path, &stat) >= 0)
    {
        state_list[num].size = stat.st_size;
        state_list[num].time = stat.st_mtime;
    }

    // Set exist
    state_list[num].exist = 1;

    sceIoClose(fd);
}

int Setting_GetStatePreviewSize(int *width, int *height)
{
    int h = STATE_PREVIEW_HEIGHT;
    if (height)
        *height = h;
    if (width)
        *width = STATE_PREVIEW_WIDTH(h);
    return 0;
}

void Setting_SetStateSelectId(int id)
{
    list_focus_pos = id;
    moveStateListPos(TYPE_MOVE_NONE);
}

int Setting_GetStateSelectId()
{
    return list_focus_pos;
}

int Setting_LoadState(int num)
{
    if (!Emu_IsGameLoaded())
    {
        EmuGameInfo info;
        MakeCurrentFilePath(info.path);
        info.type = GetCurrentFileType();
        info.state_num = num;
        return Emu_StartGame(&info);
    }

    return Emu_LoadState(num);
}

int Setting_SaveState(int num)
{
    if (!Emu_IsGameLoaded())
        return -1;

    int ret = Emu_SaveState(num);
    refreshStateItem(num);
    return ret;
}

int Setting_DeleteState(int num)
{
    int ret = Emu_DeleteState(num);
    cleanStateItem(num);
    return ret;
}

void Setting_DrawState()
{
    int layout_x = 0, layout_y = 0;
    int layout_w = 0, layout_h = 0;
    int preview_w = 0, preview_h = 0;

    Setting_GetWindowMenuLayoutPosition(&layout_x, &layout_y);
    Setting_GetWindowMenuAvailableSize(&layout_w, &layout_h);
    Setting_GetStatePreviewSize(&preview_w, &preview_h);

    int itemviews_sx = layout_x + STATE_LISTVIEW_PADDING_L;
    int itemviews_sy = layout_y + STATE_LISTVIEW_PADDING_T;
    int itemviews_w = layout_w - STATE_LISTVIEW_PADDING_L * 2;
    int itemviews_h = layout_h - STATE_LISTVIEW_PADDING_T * 2;
    int itemviews_dx = itemviews_sx + itemviews_w;
    int itemviews_dy = itemviews_sy + itemviews_h;

    int itemview_w = (itemviews_w - STATE_ITEMVIEW_MARGIN * (N_STATE_COUNTS_PER_LINE - 1)) / N_STATE_COUNTS_PER_LINE;
    int itemview_h = preview_h + STATE_ITEMVIEW_PADDING * 2;
    int itemview_x_space = itemview_w + STATE_ITEMVIEW_MARGIN;
    int itemview_y_space = itemview_h + STATE_ITEMVIEW_MARGIN;

    int itemview_x = itemviews_sx;
    int itemview_y = itemviews_sy;

    int i;
    for (i = list_top_pos; i < STATE_LIST_LEN; i++)
    {
        if (itemview_y + itemview_h <= itemviews_sy)
            goto NEXT;
        if (itemview_x >= itemviews_dx)
            goto NEXT;
        if (itemview_y >= itemviews_dy)
            break;

        uint32_t color = (i == list_focus_pos) ? STATE_ITEMVIEW_COLOR_FOCUS_BG : STATE_ITEMVIEW_COLOR_DEF_BG;
        GUI_DrawFillRectangle(itemview_x, itemview_y, itemview_w, itemview_h, color);

        int preview_x = itemview_x + STATE_ITEMVIEW_PADDING;
        int preview_y = itemview_y + STATE_ITEMVIEW_PADDING;
        int info_sx = preview_x + preview_w + STATE_INFO_MARGIN_L;
        int info_sy = preview_y + preview_h - GUI_GetLineHeight();

        if (state_list[i].exist)
        {
            // Preview
            if (state_list[i].tex)
            {
                int tex_width = GUI_GetTextureWidth(state_list[i].tex);
                int tex_height = GUI_GetTextureHeight(state_list[i].tex);
                float x_scale = (float)preview_w / (float)tex_width;
                float y_scale = (float)preview_h / (float)tex_height;
                GUI_DrawTextureScale(state_list[i].tex, preview_x, preview_y, x_scale, y_scale);
            }

            // Date & time
            char date_string[24];
            GetDateString(date_string, date_format, &state_list[i].time);
            char time_string[16];
            GetTimeString(time_string, time_format, &state_list[i].time);
            GUI_DrawTextf(info_sx, info_sy, COLOR_LITEGRAY, "%s %s", date_string, time_string);
            info_sy -= (GUI_GetLineHeight() + STATE_INFO_LINE_SPACE);
            GUI_DrawTextf(info_sx, info_sy, COLOR_LITEGRAY, "%s %d", cur_lang[LANG_STATE_EXISTENT_STATE], i);
        }
        else
        {
            // Empty
            GUI_DrawFillRectangle(preview_x, preview_y, preview_w, preview_h, STATE_PREVIEW_COLOR_BG);
            GUI_DrawTextf(info_sx, info_sy, COLOR_LITEGRAY, "%s %d", cur_lang[LANG_STATE_EMPTY_STATE], i);
        }

        // If open menu
        if (option_open && i == list_focus_pos)
        {
            int j;
            int menu_dx = itemview_x + itemview_w - STATE_ITEMVIEW_PADDING;
            int menu_sy = itemview_y + STATE_ITEMVIEW_PADDING;
            for (j = 0; j < N_MENU_ITEMS; j++)
            {
                if (state_menu[j].enable)
                {
                    uint32_t color = (j == option_focus_pos) ? COLOR_GREEN : COLOR_WHITE;
                    char *name = cur_lang[state_menu[j].name];
                    GUI_DrawTextf(menu_dx - GUI_GetTextWidth(name), menu_sy, color, name);
                    menu_sy += (GUI_GetLineHeight() + OPTION_LINE_SPACE);
                }
            }
        }

    NEXT:
        if ((i + 1) % N_STATE_COUNTS_PER_LINE == 0) // Next line
        {
            itemview_x = itemviews_sx;
            itemview_y += itemview_y_space;
        }
        else
        {
            itemview_x += itemview_x_space;
        }
    }
}

static void openMenu()
{
    option_open = 1;
    option_focus_pos = 0;

    if (state_list[list_focus_pos].exist)
    {
        state_menu[0].enable = 1;
        state_menu[2].enable = 1;
    }
    else
    {
        state_menu[0].enable = 0;
        state_menu[2].enable = 0;
    }

    if (Emu_IsGameLoaded())
        state_menu[1].enable = 1;
    else
        state_menu[1].enable = 0;

    int i;
    for (i = 0; i < N_MENU_ITEMS; i++)
    {
        if (state_menu[i].enable)
        {
            option_focus_pos = i;
            break;
        }
    }
}

static void ctrlStateList()
{
    if (hold_pad[PAD_UP] || hold_pad[PAD_LEFT_ANALOG_UP])
    {
        moveStateListPos(TYPE_MOVE_UP);
    }
    else if (hold_pad[PAD_DOWN] || hold_pad[PAD_LEFT_ANALOG_DOWN])
    {
        moveStateListPos(TYPE_MOVE_DOWN);
    }
    else if (hold_pad[PAD_LEFT] || hold_pad[PAD_LEFT_ANALOG_LEFT])
    {
        moveStateListPos(TYPE_MOVE_LEFT);
    }
    else if (hold_pad[PAD_RIGHT] || hold_pad[PAD_LEFT_ANALOG_RIGHT])
    {
        moveStateListPos(TYPE_MOVE_RIGHT);
    }
    else if (released_pad[PAD_ENTER])
    {
        openMenu();
    }
    else if (released_pad[PAD_CANCEL])
    {
        Setting_CloseMenu();
    }
}

static void ctrlOption()
{
    if (hold_pad[PAD_UP] || hold_pad[PAD_LEFT_ANALOG_UP])
    {
        int i;
        for (i = option_focus_pos - 1; i >= 0; i--)
        {
            if (state_menu[i].enable)
            {
                option_focus_pos = i;
                break;
            }
        }
    }
    else if (hold_pad[PAD_DOWN] || hold_pad[PAD_LEFT_ANALOG_DOWN])
    {
        int i;
        for (i = option_focus_pos + 1; i <= N_MENU_ITEMS - 1; i++)
        {
            if (state_menu[i].enable)
            {
                option_focus_pos = i;
                break;
            }
        }
    }
    else if (released_pad[PAD_ENTER])
    {
        switch (option_focus_pos)
        {
        case 0:
        {
            option_open = 0;
            Setting_CloseMenu();
            Setting_LoadState(list_focus_pos);
            break;
        }
        case 1:
        {
            option_open = 0;
            Setting_SaveState(list_focus_pos);
            break;
        }
        case 2:
        {
            option_open = 0;
            Setting_DeleteState(list_focus_pos);
            break;
        }
        case 3:
        {
            option_open = 0;
            break;
        }
        }
    }
    else if (released_pad[PAD_CANCEL])
    {
        option_open = 0;
    }
}

void Setting_CtrlState()
{
    if (option_open)
        ctrlOption();
    else
        ctrlStateList();
}

static int stateThreadFunc(SceSize args, void *argp)
{
    char dir_path[MAX_PATH_LENGTH];

    MakeSavestateDir(dir_path);

    SceUID dfd = sceIoDopen(dir_path);
    if (dfd >= 0)
    {
        int res = 0;

        do
        {
            SceIoDirent dir;
            memset(&dir, 0, sizeof(SceIoDirent));

            res = sceIoDread(dfd, &dir);
            if (res > 0)
            {
                if (strncmp(dir.d_name, "state-", 6) == 0)
                {
                    char num_str[3];
                    num_str[0] = dir.d_name[6];
                    num_str[1] = dir.d_name[7];
                    num_str[2] = '\0';

                    if (num_str[0] >= '0' && num_str[0] <= '9' &&
                        num_str[1] >= '0' && num_str[1] <= '9')
                    {
                        int num = strtol(num_str, 0, 10);
                        refreshStateItem(num);
                    }
                }
            }
        } while (res > 0 && !state_thread_stop);

        sceIoDclose(dfd);
    }

    sceKernelExitDeleteThread(0);
    return 0;
}

int initStateThread()
{
    int ret = 0;

    state_thread_stop = 0;
    ret = state_thid = sceKernelCreateThread("state_thread", stateThreadFunc, 0x10000100, 0x10000, 0, 0, NULL);
    if (state_thid >= 0)
        ret = sceKernelStartThread(state_thid, 0, NULL);

    return ret;
}

void deinitStateThread()
{
    state_thread_stop = 1;
    if (state_thid >= 0)
    {
        sceKernelWaitThreadEnd(state_thid, NULL, NULL);
        sceKernelDeleteThread(state_thid);
        state_thid = -1;
    }
}

int Setting_InitState()
{
    option_open = 0;

    state_list = calloc(STATE_LIST_LEN, sizeof(StateListItem));
    if (!state_list)
        return -1;

    moveStateListPos(TYPE_MOVE_NONE);
    initStateThread();

    return 0;
}

int Setting_DeinitState()
{
    deinitStateThread();
    freeStateList();

    return 0;
}
