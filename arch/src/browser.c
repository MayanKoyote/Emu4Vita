#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <psp2/kernel/threadmgr.h>

#include <vita2d.h>

#include "browser.h"
#include "gui.h"
#include "main.h"
#include "utils.h"
#include "file.h"
#include "config.h"
#include "boot.h"

#define VIEW_MARGIN 10

#define ICON_SHORT_NAME_PADDING 4
#define ICON_SHORT_NAME_HEIGHT (GUI_getLineHeight() + ICON_SHORT_NAME_PADDING * 2)
#define ICON_SHORT_NAME_COLOR 0xFFFFFFFF
#define ICON_LONG_NAME_COLOR 0xFFFFFFFF

#define ICON_MARGIN 2
#define ICON_UNFOCUS_WIDTH 120
#define ICON_UNFOCUS_HIEGHT 120
#define ICON_FOCUS_WIDTH 160
#define ICON_FOCUS_HIEGHT 160

#define ICON_MAX_STEP_COUNT 10

#define ICON_COLOR_NONE WHITE
#define ICON_TINT_COLOR 0xFFFFFFFF

#define ICON_SELVIEW_WIDTH(w) w
#define ICON_SELVIEW_HEIGHT(h) (h + ICON_SHORT_NAME_HEIGHT * (float)h / (float)ICON_FOCUS_HIEGHT)
#define ICON_SELVIEW_BORDER_SIZE 2
#define ICON_SELVIEW_BORDER_COLOR 0xF0FFFFFF
#define ICON_SELVIEW_BG_COLOR 0x9F1F1F1F

#define CORE_ITEM_PADDING 6
#define CORE_ITEM_WIDTH 400
#define CORE_ITEM_HEIGHT (GUI_getLineHeight() + CORE_ITEM_PADDING * 2)
#define CORE_ITEM_NAME_COLOR 0xFFFFFFFF
#define CORE_ITEM_SEL_NAME_COLOR SPRING_GREEN
#define CORE_ITEM_BG_COLOR 0x9F1F1F1F
#define CORE_ITEM_SEL_BG_COLOR 0xAFFF7F00

CoreEntry nes_entries[] = {
    {"FCEUmm", "fceumm", "NES"},
    {"Nestopia", "nestopia", "NES"},
};

CoreEntry snes_entries[] = {
    {"Snes9x 2002", "snes9x2002", "SNES"},
    {"Snes9x 2005", "snes9x2005", "SNES"},
    {"Snes9x 2005 Plus", "snes9x2005_plus", "SNES"},
};

CoreEntry md_entries[] = {
    {"Genesis Plus GX", "genesis_plus_gx", "MD"},
    {"PicoDrive", "picodrive", "MD"},
};

CoreEntry gba_entries[] = {
    {"gpSP", "gpsp", "GBA"},
    {"VBA Next", "vba_next", "GBA"},
};

CoreEntry gbc_entries[] = {
    {"Gambatte", "gambatte", "GBC"},
};

CoreEntry pce_entries[] = {
    {"Mednafen PCE Fast", "mednafen_pce_fast", "PCE"},
    {"Mednafen SuperGrafx", "mednafen_supergrafx", "PCE"},
};

CoreEntry ps_entries[] = {
    {"PCSX ReARMed", "pcsx_rearmed", "PS1"},
};

CoreEntry wsc_entries[] = {
    {"Mednafen Wswan", "mednafen_wswan", "WSC"},
};

CoreEntry ngp_entries[] = {
    {"Mednafen NeoPop", "mednafen_ngp", "NGP"},
};

CoreEntry fba_entries[] = {
    {"FBA Lite", "fba_lite", "FBA"},
};

SoftwareEntry software_entries[] = {
    {"NES", NULL, nes_entries, sizeof(nes_entries) / sizeof(CoreEntry), &g_config.nes_pos, {0}},
    {"SNES", NULL, snes_entries, sizeof(snes_entries) / sizeof(CoreEntry), &g_config.snes_pos, {0}},
    {"MD", NULL, md_entries, sizeof(md_entries) / sizeof(CoreEntry), &g_config.md_pos, {0}},
    {"GBA", NULL, gba_entries, sizeof(gba_entries) / sizeof(CoreEntry), &g_config.gba_pos, {0}},
    {"GBC", NULL, gbc_entries, sizeof(gbc_entries) / sizeof(CoreEntry), &g_config.gbc_pos, {0}},
    {"PCE", NULL, pce_entries, sizeof(pce_entries) / sizeof(CoreEntry), &g_config.pce_pos, {0}},
    {"PS1", NULL, ps_entries, sizeof(ps_entries) / sizeof(CoreEntry), &g_config.ps1_pos, {0}},
    {"WSC", NULL, wsc_entries, sizeof(wsc_entries) / sizeof(CoreEntry), &g_config.wsc_pos, {0}},
    {"NGP", NULL, ngp_entries, sizeof(ngp_entries) / sizeof(CoreEntry), &g_config.ngp_pos, {0}},
    {"FBA", NULL, fba_entries, sizeof(fba_entries) / sizeof(CoreEntry), &g_config.arc_pos, {0}},
};
#define N_SOFTWARE_ENTRIES (sizeof(software_entries) / sizeof(SoftwareEntry))

static int core_entries_open = 0;

static float icon_focus_x = 0, icon_focus_y = 0;

static float icon_selview_target_x = 0, icon_selview_target_y = 0;
static float icon_selview_current_x = 0, icon_selview_current_y = 0;
static float icon_selview_scroll_step = 0;

static float icon_target_sx = 0, icon_target_sy = 0;
static float icon_current_sx = 0, icon_current_sy = 0;
static float icon_scroll_step = 0;

static uint32_t icon_selview_border_color = 0;
static uint32_t icon_selview_bg_color = 0;
static float icon_selview_w = 0;
static float icon_selview_h = 0;

static uint32_t icon_tint_color = ICON_TINT_COLOR;
static uint32_t icon_short_name_color = 0;

static float core_current_x = 0, core_target_x = 0;
static float core_scroll_step = 0;
static uint32_t core_name_color = 0;
static uint32_t core_sel_name_color = 0;
static uint32_t core_bg_color = 0;
static uint32_t core_sel_color = 0;

static void makeCorePath(char *path, char *name)
{
    snprintf(path, MAX_PATH_LENGTH, "app0:%s.self", name);
}

static void makeIconPath(char *path, char *name)
{
    snprintf(path, MAX_PATH_LENGTH, "%s/%s/icon0.png", CORE_DATA_DIR, name);
}

static uint32_t makeAlphaColor(uint32_t cur, uint32_t target, float step_alpha)
{
    int cur_alpha = COLOR_GET_ALPHA(cur);
    int target_alpha = COLOR_GET_ALPHA(target);

    if (step_alpha < 1)
        step_alpha = 1;

    if (cur_alpha < target_alpha)
    {
        cur_alpha += step_alpha;
        if (cur_alpha > target_alpha)
            cur_alpha = target_alpha;
    }
    else if (cur_alpha > target_alpha)
    {
        cur_alpha -= step_alpha;
        if (cur_alpha < target_alpha)
            cur_alpha = target_alpha;
    }
    return COLOR_SET_ALPHA(target, cur_alpha);
}

static void setIconTexture(SoftwareEntry *entry, vita2d_texture *texture)
{
    if (entry->icon)
    {
        vita2d_wait_rendering_done();
        vita2d_free_texture(entry->icon);
    }
    entry->icon = texture;
}

static int loadCoreEboot(CoreEntry *entry)
{
    if (!entry)
        return -1;

    SaveConfig();

    char core_path[MAX_PATH_LENGTH];
    makeCorePath(core_path, entry->core_name);
    char assets_dir[MAX_PATH_LENGTH];
    snprintf(assets_dir, MAX_CONFIG_LINE_LENGTH, "%s/%s", CORE_DATA_DIR, entry->assets_name);

    int ret = loadCoreWithBootParams(core_path, assets_dir);

    return ret;
}

static void refreshIconScroll()
{
    if (g_config.software_pos == 0)
        icon_target_sx = icon_focus_x;
    else
        icon_target_sx = icon_focus_x - (g_config.software_pos * (ICON_UNFOCUS_WIDTH + ICON_MARGIN) + ICON_SELVIEW_BORDER_SIZE);

    if (icon_target_sx > icon_current_sx)
        icon_scroll_step = (float)(icon_target_sx - icon_current_sx) / (float)ICON_MAX_STEP_COUNT;
    else
        icon_scroll_step = (float)(icon_current_sx - icon_target_sx) / (float)ICON_MAX_STEP_COUNT;
}

static void initSoftwareEntriesLayout()
{
    icon_selview_w = ICON_SELVIEW_WIDTH(ICON_UNFOCUS_WIDTH);
    icon_selview_h = ICON_SELVIEW_HEIGHT(ICON_UNFOCUS_HIEGHT);
    icon_selview_border_color = 0;
    icon_selview_bg_color = 0;
    icon_short_name_color = 0;

    icon_focus_x = MAIN_FREE_DRAW_SX + ICON_UNFOCUS_WIDTH * 0.8f + ICON_MARGIN;
    icon_focus_y = MAIN_FREE_DRAW_SY + (MAIN_FREE_DRAW_HEIGHT - ICON_FOCUS_HIEGHT) / 5;

    icon_selview_current_x = icon_selview_target_x = icon_focus_x;
    icon_selview_current_y = icon_selview_target_y = icon_focus_y;

    icon_current_sx = icon_target_sx = icon_focus_x;
    icon_current_sy = icon_target_sy = icon_focus_y;

    // Set icon layout
    int i;
    for (i = 0; i < N_SOFTWARE_ENTRIES; i++)
    {
        IconLayout *layout = &software_entries[i].layout;
        layout->w = ICON_UNFOCUS_WIDTH;
        layout->h = ICON_UNFOCUS_HIEGHT;
        layout->margin_r = ICON_MARGIN;
    }

    refreshIconScroll();
}

static void updateSoftwareEntriesLayout()
{
    if (icon_current_sx < icon_target_sx)
    {
        icon_current_sx += icon_scroll_step;
        if (icon_current_sx > icon_target_sx)
            icon_current_sx = icon_target_sx;
    }
    else if (icon_current_sx > icon_target_sx)
    {
        icon_current_sx -= icon_scroll_step;
        if (icon_current_sx < icon_target_sx)
            icon_current_sx = icon_target_sx;
    }

    if (icon_selview_current_x < icon_selview_target_x)
    {
        icon_selview_current_x += icon_selview_scroll_step;
        if (icon_selview_current_x > icon_selview_target_x)
            icon_selview_current_x = icon_selview_target_x;
    }
    else if (icon_selview_current_x > icon_selview_target_x)
    {
        icon_selview_current_x -= icon_selview_scroll_step;
        if (icon_selview_current_x < icon_selview_target_x)
            icon_selview_current_x = icon_selview_target_x;
    }

    int target_w, target_h;
    float icon_w_scale_step = (float)(ICON_FOCUS_WIDTH - ICON_UNFOCUS_WIDTH) / (float)ICON_MAX_STEP_COUNT;
    float icon_h_scale_step = (float)(ICON_FOCUS_HIEGHT - ICON_UNFOCUS_HIEGHT) / (float)ICON_MAX_STEP_COUNT;
    float icon_margin_scale_step = (float)ICON_SELVIEW_BORDER_SIZE / (float)ICON_MAX_STEP_COUNT;

    int i;
    for (i = 0; i < N_SOFTWARE_ENTRIES; i++)
    {
        IconLayout *layout = &software_entries[i].layout;

        target_w = ICON_UNFOCUS_WIDTH;
        target_h = ICON_UNFOCUS_HIEGHT;

        if (i == g_config.software_pos)
        {
            target_w = ICON_FOCUS_WIDTH;
            target_h = ICON_FOCUS_HIEGHT;
        }

        // Scale icon width
        if (layout->w < target_w)
        {
            layout->w += icon_w_scale_step;
            if (layout->w > target_w)
                layout->w = target_w;
        }
        else if (layout->w > target_w)
        {
            layout->w -= icon_w_scale_step;
            if (layout->w < target_w)
                layout->w = target_w;
        }

        // Scale icon height
        if (layout->h < target_h)
        {
            layout->h += icon_h_scale_step;
            if (layout->h > target_h)
                layout->h = target_h;
        }
        else if (layout->h > target_h)
        {
            layout->h -= icon_h_scale_step;
            if (layout->h < target_h)
                layout->h = target_h;
        }

        // Scale icon margin right
        int target_margin_r = ICON_MARGIN;
        if (i == g_config.software_pos || i == g_config.software_pos - 1)
            target_margin_r += ICON_SELVIEW_BORDER_SIZE;

        if (layout->margin_r < target_margin_r)
        {
            layout->margin_r += icon_margin_scale_step;
            if (layout->margin_r > target_margin_r)
                layout->margin_r = target_margin_r;
        }
        else if (layout->margin_r > target_margin_r)
        {
            layout->margin_r -= icon_margin_scale_step;
            if (layout->margin_r < target_margin_r)
                layout->margin_r = target_margin_r;
        }
    }

    IconLayout *focus_layout = &software_entries[g_config.software_pos].layout;

    target_w = ICON_SELVIEW_WIDTH(focus_layout->w);
    target_h = ICON_SELVIEW_HEIGHT(focus_layout->h);
    icon_w_scale_step = (float)(ICON_SELVIEW_WIDTH(ICON_FOCUS_WIDTH) - ICON_SELVIEW_WIDTH(ICON_UNFOCUS_WIDTH)) / (float)ICON_MAX_STEP_COUNT;
    icon_h_scale_step = (float)(ICON_SELVIEW_HEIGHT(ICON_FOCUS_HIEGHT) - ICON_SELVIEW_HEIGHT(ICON_UNFOCUS_HIEGHT)) / (float)ICON_MAX_STEP_COUNT;

    // Scale selview width
    if (icon_selview_w < target_w)
    {
        icon_selview_w += icon_w_scale_step;
        if (icon_selview_w > target_w)
            icon_selview_w = target_w;
    }
    else if (icon_selview_w > target_w)
    {
        icon_selview_w -= icon_w_scale_step;
        if (icon_selview_w < target_w)
            icon_selview_w = target_w;
    }
    // Scale selview height
    if (icon_selview_h < target_h)
    {
        icon_selview_h += icon_h_scale_step;
        if (icon_selview_h > target_h)
            icon_selview_h = target_h;
    }
    else if (icon_selview_h > target_h)
    {
        icon_selview_h -= icon_h_scale_step;
        if (icon_selview_h < target_h)
            icon_selview_h = target_h;
    }

    if (focus_layout->h == ICON_FOCUS_HIEGHT)
    {
        icon_short_name_color = makeAlphaColor(icon_short_name_color, ICON_SHORT_NAME_COLOR, COLOR_GET_ALPHA(ICON_SHORT_NAME_COLOR) / (float)ICON_MAX_STEP_COUNT);
    }

    if (core_entries_open)
        icon_tint_color = makeAlphaColor(icon_tint_color, COLOR_SET_ALPHA(ICON_TINT_COLOR, 0), COLOR_GET_ALPHA(ICON_TINT_COLOR) / (float)ICON_MAX_STEP_COUNT);
    else
        icon_tint_color = makeAlphaColor(icon_tint_color, ICON_TINT_COLOR, COLOR_GET_ALPHA(ICON_TINT_COLOR) / (float)ICON_MAX_STEP_COUNT);

    icon_selview_border_color = makeAlphaColor(icon_selview_border_color, ICON_SELVIEW_BORDER_COLOR, COLOR_GET_ALPHA(ICON_SELVIEW_BORDER_COLOR) / (float)ICON_MAX_STEP_COUNT);
    icon_selview_bg_color = makeAlphaColor(icon_selview_bg_color, ICON_SELVIEW_BG_COLOR, COLOR_GET_ALPHA(ICON_SELVIEW_BG_COLOR) / (float)ICON_MAX_STEP_COUNT);
}

static void moveSoftwareEntriesPos(int type)
{
    int old_focus_pos = g_config.software_pos;
    int new_focus_pos = g_config.software_pos;

    if (type == TYPE_MOVE_LEFT)
        new_focus_pos--;
    else if (type == TYPE_MOVE_RIGHT)
        new_focus_pos++;
    if (new_focus_pos < 0)
        new_focus_pos = 0;
    if (new_focus_pos > N_SOFTWARE_ENTRIES - 1)
        new_focus_pos = N_SOFTWARE_ENTRIES - 1;

    if (old_focus_pos != new_focus_pos)
    {
        g_config.software_pos = new_focus_pos;
        refreshIconScroll();

        icon_selview_w = ICON_SELVIEW_WIDTH(ICON_UNFOCUS_WIDTH);
        icon_selview_h = ICON_SELVIEW_HEIGHT(ICON_UNFOCUS_HIEGHT);
        if (new_focus_pos < old_focus_pos)
            icon_selview_current_x = icon_focus_x + (ICON_FOCUS_WIDTH - ICON_UNFOCUS_WIDTH);
        if (icon_selview_target_x > icon_selview_current_x)
            icon_selview_scroll_step = (float)(icon_selview_target_x - icon_selview_current_x) / (float)ICON_MAX_STEP_COUNT;
        else
            icon_selview_scroll_step = (float)(icon_selview_current_x - icon_selview_target_x) / (float)ICON_MAX_STEP_COUNT;

        icon_selview_border_color = 0;
        icon_selview_bg_color = 0;
        icon_short_name_color = 0;
    }
}

static void updateCoreEntriesLayout()
{
    if (core_current_x < core_target_x)
    {
        core_current_x += core_scroll_step;
        if (core_current_x > core_target_x)
            core_current_x = core_target_x;
    }
    else if (core_current_x > core_target_x)
    {
        core_current_x -= core_scroll_step;
        if (core_current_x < core_target_x)
            core_current_x = core_target_x;
    }

    uint32_t target_name_color;
    uint32_t target_sel_name_color;
    uint32_t target_bg_color;
    uint32_t target_sel_bg_color;
    if (core_entries_open)
    {
        target_name_color = CORE_ITEM_NAME_COLOR;
        target_sel_name_color = CORE_ITEM_SEL_NAME_COLOR;
        target_bg_color = CORE_ITEM_BG_COLOR;
        target_sel_bg_color = CORE_ITEM_SEL_BG_COLOR;
    }
    else
    {
        target_name_color = COLOR_SET_ALPHA(CORE_ITEM_NAME_COLOR, 0);
        target_sel_name_color = COLOR_SET_ALPHA(CORE_ITEM_SEL_NAME_COLOR, 0);
        target_bg_color = COLOR_SET_ALPHA(CORE_ITEM_BG_COLOR, 0);
        target_sel_bg_color = COLOR_SET_ALPHA(CORE_ITEM_SEL_BG_COLOR, 0);
    }
    core_name_color = makeAlphaColor(core_name_color, target_name_color, COLOR_GET_ALPHA(CORE_ITEM_NAME_COLOR) / (float)ICON_MAX_STEP_COUNT);
    core_sel_name_color = makeAlphaColor(core_sel_name_color, target_sel_name_color, COLOR_GET_ALPHA(CORE_ITEM_NAME_COLOR) / (float)ICON_MAX_STEP_COUNT);
    core_bg_color = makeAlphaColor(core_bg_color, target_bg_color, COLOR_GET_ALPHA(CORE_ITEM_BG_COLOR) / (float)ICON_MAX_STEP_COUNT);
    core_sel_color = makeAlphaColor(core_sel_color, target_sel_bg_color, COLOR_GET_ALPHA(CORE_ITEM_SEL_BG_COLOR) / (float)ICON_MAX_STEP_COUNT);
}

static void moveCoreEntriesPos(int type)
{
    SoftwareEntry *software_entry = &software_entries[g_config.software_pos];
    int new_focus_pos = *software_entry->entries_pos;

    if (type == TYPE_MOVE_UP)
        new_focus_pos--;
    else if (type == TYPE_MOVE_DOWN)
        new_focus_pos++;
    if (new_focus_pos < 0)
        new_focus_pos = 0;
    if (new_focus_pos > software_entry->n_entries - 1)
        new_focus_pos = software_entry->n_entries - 1;

    *software_entry->entries_pos = new_focus_pos;
}

static void openCoreEntries()
{
    moveCoreEntriesPos(TYPE_MOVE_NONE);
    refreshIconScroll();

    int x_scroll_size = ICON_UNFOCUS_WIDTH * 0.5f;

    icon_selview_target_x = icon_focus_x - x_scroll_size;
    if (icon_selview_target_x > icon_selview_current_x)
        icon_selview_scroll_step = (float)(icon_selview_target_x - icon_selview_current_x) / (float)ICON_MAX_STEP_COUNT;
    else
        icon_selview_scroll_step = (float)(icon_selview_current_x - icon_selview_target_x) / (float)ICON_MAX_STEP_COUNT;

    icon_target_sx -= x_scroll_size;
    if (icon_target_sx > icon_current_sx)
        icon_scroll_step = (float)(icon_target_sx - icon_current_sx) / (float)ICON_MAX_STEP_COUNT;
    else
        icon_scroll_step = (float)(icon_current_sx - icon_target_sx) / (float)ICON_MAX_STEP_COUNT;

    core_target_x = icon_selview_target_x + ICON_FOCUS_WIDTH + 20;
    core_current_x = icon_focus_x + ICON_FOCUS_WIDTH + 40;
    core_scroll_step = (float)(core_current_x - core_target_x) / (float)ICON_MAX_STEP_COUNT;

    core_name_color = 0;
    core_bg_color = 0;
    core_sel_color = 0;

    core_entries_open = 1;
}

static void closeCoreEntries()
{
    refreshIconScroll();

    icon_selview_target_x = icon_focus_x;
    if (icon_selview_target_x > icon_selview_current_x)
        icon_selview_scroll_step = (float)(icon_selview_target_x - icon_selview_current_x) / (float)ICON_MAX_STEP_COUNT;
    else
        icon_selview_scroll_step = (float)(icon_selview_current_x - icon_selview_target_x) / (float)ICON_MAX_STEP_COUNT;

    core_entries_open = 0;
    core_target_x = icon_focus_x + ICON_FOCUS_WIDTH + 40;
    core_scroll_step = (float)(core_target_x - core_current_x) / (float)ICON_MAX_STEP_COUNT;
}

static int drawCoreEntries()
{
    if (!core_entries_open && core_current_x == core_target_x)
        return 0;

    updateCoreEntriesLayout();

    SoftwareEntry *software_entry = &software_entries[g_config.software_pos];
    CoreEntry *core_entries = software_entry->entries;

    int x = core_current_x;
    int y = icon_focus_y;
    uint32_t name_color;

    int i;
    for (i = 0; i < software_entry->n_entries; i++)
    {
        vita2d_draw_rectangle(x, y, CORE_ITEM_WIDTH, CORE_ITEM_HEIGHT, CORE_ITEM_BG_COLOR);
        name_color = core_name_color;

        if (i == *software_entry->entries_pos)
        {
            name_color = core_sel_name_color;
            vita2d_draw_rectangle(x, y, CORE_ITEM_WIDTH, CORE_ITEM_HEIGHT, CORE_ITEM_SEL_BG_COLOR);
        }

        GUI_drawText(x + CORE_ITEM_PADDING, y + CORE_ITEM_PADDING, name_color, core_entries[i].desc);

        y += (CORE_ITEM_HEIGHT + 2);
    }

    return 0;
}

static int ctrlCoreEntries()
{
    if (hold_pad[PAD_UP] || hold2_pad[PAD_LEFT_ANALOG_UP])
    {
        moveCoreEntriesPos(TYPE_MOVE_UP);
    }
    else if (hold_pad[PAD_DOWN] || hold2_pad[PAD_LEFT_ANALOG_DOWN])
    {
        moveCoreEntriesPos(TYPE_MOVE_DOWN);
    }

    if (pressed_pad[PAD_CANCEL])
    {
        closeCoreEntries();
    }

    if (pressed_pad[PAD_ENTER])
    {
        SoftwareEntry *software_entry = &software_entries[g_config.software_pos];
        CoreEntry *core_entry = &software_entry->entries[*software_entry->entries_pos];
        loadCoreEboot(core_entry);
    }

    return 0;
}

static int drawSoftwareEntries()
{
    updateSoftwareEntriesLayout();

    SoftwareEntry *entry;
    IconLayout *layout;

    SoftwareEntry *focus_entry = &software_entries[g_config.software_pos];

    // Draw selview bg
    vita2d_draw_rectangle(icon_selview_current_x, icon_selview_current_y, icon_selview_w, icon_selview_h, icon_selview_bg_color);

    // Draw icon
    int x = icon_current_sx;
    int y = icon_current_sy;
    float x_scale, y_scale;

    int i;
    for (i = 0; i < N_SOFTWARE_ENTRIES; i++)
    {
        entry = &software_entries[i];
        layout = &entry->layout;

        if (x > MAIN_FREE_DRAW_DX)
            break;
        if (x + layout->w < 0)
            goto NEXT;

        uint32_t tint_color = icon_tint_color;
        if (i == g_config.software_pos)
            tint_color = ICON_TINT_COLOR;

        if (COLOR_GET_ALPHA(tint_color) == 0)
            goto NEXT;

        if (entry->icon)
        {
            x_scale = (float)layout->w / (float)vita2d_texture_get_width(entry->icon);
            y_scale = (float)layout->h / (float)vita2d_texture_get_height(entry->icon);
            vita2d_draw_texture_tint_scale(entry->icon, x, y, x_scale, y_scale, tint_color);
        }
        else
        {
            vita2d_draw_rectangle(x, y, layout->w, layout->h, ICON_COLOR_NONE & tint_color);
        }

    NEXT:
        x += (layout->w + layout->margin_r);
    }

    // Draw selview border
    vita2d_draw_empty_rectangle(icon_selview_current_x - ICON_SELVIEW_BORDER_SIZE, icon_selview_current_y - ICON_SELVIEW_BORDER_SIZE,
                                icon_selview_w + ICON_SELVIEW_BORDER_SIZE * 2, icon_selview_h + ICON_SELVIEW_BORDER_SIZE * 2,
                                ICON_SELVIEW_BORDER_SIZE, icon_selview_border_color);
    // Draw icon short name
    int short_name_x = icon_selview_current_x + (icon_selview_w - GUI_getTextWidth(focus_entry->short_name)) / 2;
    int short_name_y = icon_selview_current_y + focus_entry->layout.h + (icon_selview_h - focus_entry->layout.h - GUI_getLineHeight()) / 2;
    GUI_drawText(short_name_x, short_name_y, icon_short_name_color, focus_entry->short_name);

    return 0;
}

static int ctrlSoftwareEntries()
{
    if (hold_pad[PAD_LEFT] || hold2_pad[PAD_LEFT_ANALOG_LEFT])
    {
        moveSoftwareEntriesPos(TYPE_MOVE_LEFT);
    }
    else if (hold_pad[PAD_RIGHT] || hold2_pad[PAD_LEFT_ANALOG_RIGHT])
    {
        moveSoftwareEntriesPos(TYPE_MOVE_RIGHT);
    }

    if (pressed_pad[PAD_ENTER])
    {
        if (icon_current_sx == icon_target_sx)
            openCoreEntries();
    }

    return 0;
}

int drawBrowser()
{
    drawCoreEntries();
    drawSoftwareEntries();

    return 0;
}

int ctrlBrowser()
{
    if (core_entries_open)
        ctrlCoreEntries();
    else
        ctrlSoftwareEntries();

    return 0;
}

static int iconsThreadCallback(SceSize args, void *argp)
{
    vita2d_texture *texture;
    char path[MAX_PATH_LENGTH];

    int i;
    for (i = 0; i < N_SOFTWARE_ENTRIES; i++)
    {
        makeIconPath(path, software_entries[i].short_name);
        texture = vita2d_load_PNG_file(path);
        setIconTexture(&software_entries[i], texture);
    }

    sceKernelExitDeleteThread(0);
    return 0;
}

static void initIconsThread()
{
    SceUID thid = sceKernelCreateThread("init_icons_thread", iconsThreadCallback, 0x10000100, 0x10000, 0, 0, NULL);
    if (thid >= 0)
        sceKernelStartThread(thid, 0, NULL);
}

int initBrowser()
{
    LoadConfig();
    initIconsThread();
    initSoftwareEntriesLayout();
    moveSoftwareEntriesPos(TYPE_MOVE_NONE);

    return 0;
}
