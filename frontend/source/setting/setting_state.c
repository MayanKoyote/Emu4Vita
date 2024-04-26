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
#include "app.h"

#define STATE_LISTVIEW_PADDING 8

#define STATE_ITEMVIEW_PADDING 6
#define STATE_ITEMVIEW_MARGIN STATE_LISTVIEW_PADDING

#define OPTION_LINE_SPACE 4

#define STATE_PREVIEW_HEIGHT ((GUI_GetLineHeight() + OPTION_LINE_SPACE) * 4 - OPTION_LINE_SPACE)
#define STATE_PREVIEW_WIDTH(h) (h * 4.0f / 3.0f)
#define STATE_ITEMVIEW_HEIGHT (STATE_PREVIEW_HEIGHT + STATE_ITEMVIEW_PADDING * 2)

#define STATE_ITEMVIEW_COLOR_DEF_BG COLOR_ALPHA(COLOR_DARKGRAY, 0xAF)
#define STATE_ITEMVIEW_COLOR_FOCUS_BG GUI_DEF_COLOR_FOCUS

#define N_STATE_ENTRIES 30
#define N_STATE_COUNTS_PER_LINE 2

#define STATE_INFO_MARGIN_L 6
#define STATE_INFO_LINE_SPACE 4

#define STATE_PREVIEW_COLOR_BG COLOR_ALPHA(COLOR_GRAY, 0xAF)

typedef struct
{
    int exist;
    GUI_Texture *tex;
    SceOff size;
    SceDateTime time;
} StateEntry;

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

static StateEntry state_entries[N_STATE_ENTRIES] = {0};
static SceUID state_thid = -1;
static int state_finish = 0;

static int listview_wrap_h = 0;
static float listview_current_scroll_y = 0;
static float listview_target_scroll_y = 0;
static float listview_scroll_step = 0;
static int entries_pos = 0;
static int menu_open = 0;
static int menu_pos = 0;

static void updateStateSrcoll()
{
    if (listview_current_scroll_y != listview_target_scroll_y)
    {
        int scroll_y = listview_current_scroll_y + listview_scroll_step;
        if (listview_current_scroll_y < listview_target_scroll_y)
        {
            if (scroll_y > listview_target_scroll_y)
                scroll_y = listview_target_scroll_y;
        }
        else
        {
            if (scroll_y < listview_target_scroll_y)
                scroll_y = listview_target_scroll_y;
        }
        listview_current_scroll_y = scroll_y;
    }
}

static void updateStateLayout()
{
    int layout_h = 0;
    SettingWindow_GetMenuAvailableSize(NULL, &layout_h);

    int n_line = (N_STATE_ENTRIES + N_STATE_COUNTS_PER_LINE - 1) / N_STATE_COUNTS_PER_LINE;
    int itemview_h = STATE_ITEMVIEW_HEIGHT;
    int itemview_y_space = itemview_h + STATE_ITEMVIEW_MARGIN;
    int itemviews_h = layout_h - STATE_LISTVIEW_PADDING * 2;
    int itemviews_wrap_h = itemview_y_space * n_line - STATE_ITEMVIEW_MARGIN;

    int scroll_y = 0 - (entries_pos / N_STATE_COUNTS_PER_LINE) * itemview_y_space;
    scroll_y += (itemviews_h / 2 - itemview_h / 2);

    int max_srcoll_y = 0;
    int min_scroll_y = MIN(itemviews_h - itemviews_wrap_h, 0);

    if (scroll_y < min_scroll_y)
        scroll_y = min_scroll_y;
    if (scroll_y > max_srcoll_y)
        scroll_y = max_srcoll_y;

    listview_target_scroll_y = scroll_y;
    listview_scroll_step = (listview_target_scroll_y - listview_current_scroll_y) / 10;
    listview_wrap_h = itemviews_wrap_h + STATE_LISTVIEW_PADDING * 2;
}

static void cleanStateEntry(int num)
{
    if (num < 0 || num >= N_STATE_ENTRIES)
        return;

    if (state_entries[num].exist)
    {
        GUI_LockDrawMutex();
        state_entries[num].exist = 0;
        if (state_entries[num].tex)
        {
            GUI_DestroyTexture(state_entries[num].tex);
            state_entries[num].tex = NULL;
        }
        GUI_UnlockDrawMutex();
    }
}

static void cleanStateEntries()
{
    GUI_LockDrawMutex();
    int i;
    for (i = 0; i < N_STATE_ENTRIES; i++)
    {
        if (state_entries[i].exist)
        {
            state_entries[i].exist = 0;
            if (state_entries[i].tex)
            {
                GUI_DestroyTexture(state_entries[i].tex);
                state_entries[i].tex = NULL;
            }
        }
    }
    GUI_UnlockDrawMutex();
}

static void refreshStateEntry(int num)
{
    char path[MAX_PATH_LENGTH];

    if (num < 0 || num >= N_STATE_ENTRIES)
        return;

    cleanStateEntry(num);

    MakeSavestatePath(path, num);
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
    state_entries[num].tex = GUI_CreateTextureFormat(header.preview_width, header.preview_height, SCEEENSHOT_PIXEL_FORMAT);
    if (state_entries[num].tex)
    {
        sceIoLseek(fd, header.preview_offset, SCE_SEEK_SET);
        sceIoRead(fd, GUI_GetTextureDatap(state_entries[num].tex), header.preview_size);
    }

    // Set file stat
    SceIoStat stat;
    memset(&stat, 0, sizeof(SceIoStat));
    if (sceIoGetstat(path, &stat) >= 0)
    {
        state_entries[num].size = stat.st_size;
        state_entries[num].time = stat.st_mtime;
    }

    // Set exist
    state_entries[num].exist = 1;

    sceIoClose(fd);
}

int Setting_LoadState(int num)
{
    if (!Emu_IsGameLoaded())
    {
        EmuGameInfo info;
        MakeCurrentGamePath(info.path);
        info.state_num = num;
        return Emu_StartGame(&info);
    }

    Emu_SetGameEventAction(EMU_GAME_EVENT_ACTION_LOAD_STATE);
    Emu_WaitGameEventDone();
    Emu_CleanAudio();
    return 0;
}

int Setting_SaveState(int num)
{
    if (!Emu_IsGameLoaded())
        return -1;

    Emu_SetGameEventAction(EMU_GAME_EVENT_ACTION_SAVE_STATE);
    Emu_WaitGameEventDone();
    refreshStateEntry(entries_pos);
    return 0;
}

int Setting_DeleteState(int num)
{
    Emu_DeleteState(num);
    cleanStateEntry(num);
    return 0;
}

void Setting_DrawState()
{
    int layout_x = 0, layout_y = 0;
    int layout_w = 0, layout_h = 0;
    int preview_w = 0, preview_h = 0;

    updateStateSrcoll();

    SettingWindow_GetMenuLayoutPosition(&layout_x, &layout_y);
    SettingWindow_GetMenuAvailableSize(&layout_w, &layout_h);
    Setting_GetStatePreviewSize(&preview_w, &preview_h);

    GUI_SetClipping(layout_x, layout_y, layout_w, layout_h);

    int itemviews_sx = layout_x + STATE_LISTVIEW_PADDING;
    int itemviews_sy = layout_y + STATE_LISTVIEW_PADDING;
    int itemviews_w = layout_w - STATE_LISTVIEW_PADDING * 2;
    int itemviews_h = layout_h - STATE_LISTVIEW_PADDING * 2;
    int itemviews_wrap_h = listview_wrap_h - STATE_LISTVIEW_PADDING * 2;
    int itemviews_dx = itemviews_sx + itemviews_w;
    int itemviews_dy = itemviews_sy + itemviews_h;

    int itemview_w = (itemviews_w - STATE_ITEMVIEW_MARGIN * (N_STATE_COUNTS_PER_LINE - 1)) / N_STATE_COUNTS_PER_LINE;
    int itemview_h = preview_h + STATE_ITEMVIEW_PADDING * 2;
    int itemview_x_space = itemview_w + STATE_ITEMVIEW_MARGIN;
    int itemview_y_space = itemview_h + STATE_ITEMVIEW_MARGIN;

    int itemview_x = itemviews_sx;
    int itemview_y = itemviews_sy + listview_current_scroll_y;

    GUI_SetClipping(itemviews_sx, itemviews_sy, itemviews_w, itemviews_h);

    int i;
    for (i = 0; i < N_STATE_ENTRIES; i++)
    {
        if (itemview_y + itemview_h <= itemviews_sy)
            goto NEXT;
        if (itemview_x >= itemviews_dx)
            goto NEXT;
        if (itemview_y >= itemviews_dy)
            break;

        uint32_t color = (i == entries_pos) ? STATE_ITEMVIEW_COLOR_FOCUS_BG : STATE_ITEMVIEW_COLOR_DEF_BG;
        GUI_DrawFillRectangle(itemview_x, itemview_y, itemview_w, itemview_h, color);

        int preview_x = itemview_x + STATE_ITEMVIEW_PADDING;
        int preview_y = itemview_y + STATE_ITEMVIEW_PADDING;
        int info_sx = preview_x + preview_w + STATE_INFO_MARGIN_L;
        int info_sy = preview_y + preview_h - GUI_GetLineHeight();

        if (state_entries[i].exist)
        {
            // Preview
            if (state_entries[i].tex)
            {
                int tex_width = GUI_GetTextureWidth(state_entries[i].tex);
                int tex_height = GUI_GetTextureHeight(state_entries[i].tex);
                float x_scale = (float)preview_w / (float)tex_width;
                float y_scale = (float)preview_h / (float)tex_height;
                GUI_DrawTextureScale(state_entries[i].tex, preview_x, preview_y, x_scale, y_scale);
            }

            // Date & time
            char date_string[24];
            GetDateString(date_string, system_date_format, &state_entries[i].time);
            char time_string[16];
            GetTimeString(time_string, system_time_format, &state_entries[i].time);
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
        if (menu_open && i == entries_pos)
        {
            int j;
            int menu_dx = itemview_x + itemview_w - STATE_ITEMVIEW_PADDING;
            int menu_sy = itemview_y + STATE_ITEMVIEW_PADDING;
            for (j = 0; j < N_MENU_ITEMS; j++)
            {
                if (state_menu[j].enable)
                {
                    uint32_t color = (j == menu_pos) ? COLOR_GREEN : COLOR_WHITE;
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

    GUI_UnsetClipping();

    // Draw scrollbar
    int track_x = layout_x + layout_w - GUI_DEF_SCROLLBAR_SIZE;
    int track_y = layout_y;
    int track_h = layout_h;
    GUI_DrawVerticalScrollbar(track_x, track_y, track_h, itemviews_wrap_h, itemviews_h, 0 - listview_target_scroll_y, 0);

    GUI_UnsetClipping();
}

static void moveStateEntriesPos(int type)
{
    int pos = entries_pos;

    if ((type == TYPE_MOVE_UP))
    {
        if (pos >= N_STATE_COUNTS_PER_LINE)
            pos -= N_STATE_COUNTS_PER_LINE;
    }
    else if ((type == TYPE_MOVE_DOWN))
    {
        if (pos < N_STATE_ENTRIES - N_STATE_COUNTS_PER_LINE)
            pos += N_STATE_COUNTS_PER_LINE;
    }
    else if (type == TYPE_MOVE_LEFT)
    {
        if (pos > 0)
            pos--;
    }
    else if (type == TYPE_MOVE_RIGHT)
    {
        if (pos < N_STATE_ENTRIES - 1)
            pos++;
    }

    if (pos > N_STATE_ENTRIES - 1)
        pos = N_STATE_ENTRIES - 1;
    if (pos < 0)
        pos = 0;

    entries_pos = pos;
    updateStateLayout();
}

static void openStateMenu()
{
    menu_open = 1;
    menu_pos = 0;

    if (state_entries[entries_pos].exist)
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
            menu_pos = i;
            break;
        }
    }
}

static void ctrlStateEntries()
{
    if (hold_pad[PAD_UP] || hold_pad[PAD_LEFT_ANALOG_UP])
    {
        moveStateEntriesPos(TYPE_MOVE_UP);
    }
    else if (hold_pad[PAD_DOWN] || hold_pad[PAD_LEFT_ANALOG_DOWN])
    {
        moveStateEntriesPos(TYPE_MOVE_DOWN);
    }
    else if (hold_pad[PAD_LEFT] || hold_pad[PAD_LEFT_ANALOG_LEFT])
    {
        moveStateEntriesPos(TYPE_MOVE_LEFT);
    }
    else if (hold_pad[PAD_RIGHT] || hold_pad[PAD_LEFT_ANALOG_RIGHT])
    {
        moveStateEntriesPos(TYPE_MOVE_RIGHT);
    }
    else if (released_pad[PAD_ENTER])
    {
        openStateMenu();
    }
    else if (released_pad[PAD_CANCEL])
    {
        Setting_CloseMenu();
    }
}

static void ctrlStateMenu()
{
    if (hold_pad[PAD_UP] || hold_pad[PAD_LEFT_ANALOG_UP])
    {
        int i;
        for (i = menu_pos - 1; i >= 0; i--)
        {
            if (state_menu[i].enable)
            {
                menu_pos = i;
                break;
            }
        }
    }
    else if (hold_pad[PAD_DOWN] || hold_pad[PAD_LEFT_ANALOG_DOWN])
    {
        int i;
        for (i = menu_pos + 1; i <= N_MENU_ITEMS - 1; i++)
        {
            if (state_menu[i].enable)
            {
                menu_pos = i;
                break;
            }
        }
    }
    else if (released_pad[PAD_ENTER])
    {
        switch (menu_pos)
        {
        case 0:
        {
            menu_open = 0;
            Setting_LoadState(entries_pos);
            Setting_CloseMenu();
            break;
        }
        case 1:
        {
            menu_open = 0;
            Setting_SaveState(entries_pos);
            break;
        }
        case 2:
        {
            menu_open = 0;
            Setting_DeleteState(entries_pos);
            break;
        }
        case 3:
        {
            menu_open = 0;
            break;
        }
        }
    }
    else if (released_pad[PAD_CANCEL])
    {
        menu_open = 0;
    }
}

void Setting_CtrlState()
{
    if (menu_open)
        ctrlStateMenu();
    else
        ctrlStateEntries();
}

static int StateThreadEntry(SceSize args, void *argp)
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
                        refreshStateEntry(num);
                    }
                }
            }
        } while (res > 0 && !state_finish);

        sceIoDclose(dfd);
    }

    state_thid = -1;
    sceKernelExitDeleteThread(0);
    return 0;
}

static int startStateThread()
{
    int ret = 0;

    if (state_thid < 0)
        ret = state_thid = sceKernelCreateThread("setting_state_thread", StateThreadEntry, 0x10000100, SCE_KERNEL_CPU_MASK_USER_0, 0, 0, NULL);
    if (state_thid >= 0)
    {
        state_finish = 0;
        ret = sceKernelStartThread(state_thid, 0, NULL);
    }

    return ret;
}

static void finishStateThread()
{
    state_finish = 1;
    if (state_thid >= 0)
    {
        sceKernelWaitThreadEnd(state_thid, NULL, NULL);
        sceKernelDeleteThread(state_thid);
        state_thid = -1;
    }
}

int Setting_InitState()
{
    menu_open = 0;
    moveStateEntriesPos(TYPE_MOVE_NONE);
    startStateThread();

    return 0;
}

int Setting_DeinitState()
{
    finishStateThread();
    cleanStateEntries();

    return 0;
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
    entries_pos = id;
    moveStateEntriesPos(TYPE_MOVE_NONE);
    listview_current_scroll_y = listview_target_scroll_y;
}

int Setting_GetStateSelectId()
{
    return entries_pos;
}
