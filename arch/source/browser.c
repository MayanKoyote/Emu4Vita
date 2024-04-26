#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <psp2/kernel/processmgr.h>

#include <vita2d.h>

#include "browser.h"
#include "gui.h"
#include "utils.h"
#include "file.h"
#include "config.h"
#include "boot.h"
#include "app.h"
#include "wallpaper.h"

#define NES_SHOR_NAME "NES"
#define SNES_SHOR_NAME "SNES"
#define MD_SHOR_NAME "MD"
#define GBA_SHOR_NAME "GBA"
#define GBC_SHOR_NAME "GBC"
#define PCE_SHOR_NAME "PCE"
#define PS1_SHOR_NAME "PS1"
#define WSC_SHOR_NAME "WSC"
#define NGP_SHOR_NAME "NGP"
#define ARC_SHOR_NAME "ARC"

// ICON
#define ICON_UNFOCUS_MARGIN 3
#define ICON_FOCUS_MARGIN 8
#define ICON_UNFOCUS_WIDTH 120
#define ICON_UNFOCUS_HIEGHT 120
#define ICON_FOCUS_WIDTH 160
#define ICON_FOCUS_HIEGHT 160

#define ICON_SHORT_NAME_PADDING 6
#define ICON_SHORT_NAME_HEIGHT (GUI_GetLineHeight() + ICON_SHORT_NAME_PADDING * 2)
#define ICON_SHORT_NAME_GET_HEIGHT(icon_h) (ICON_SHORT_NAME_HEIGHT * (icon_h - ICON_UNFOCUS_HIEGHT) / (float)(ICON_FOCUS_HIEGHT - ICON_UNFOCUS_HIEGHT))
#define ICON_SHORT_NAME_COLOR 0xFFFFFFFF
#define ICON_SHORT_NAME_BG_COLOR 0x9FC05012
#define ICON_LONG_NAME_COLOR 0xFFFFFFFF

#define ICON_EMPTY_COLOR WHITE
#define ICON_TINT_COLOR 0xFFFFFFFF

#define ICON_SELVIEW_BORDER_SIZE 2
#define ICON_SELVIEW_BORDER_COLOR 0xE0F0F0F0
#define ICON_SELVIEW_BG_COLOR 0x3F1F1F1F

#define MAX_ICON_STEP_COUNT 10
#define MAX_BORDER_COLOR_STEP_COUNT 30

// CORE entries
#define CORE_ITEMVIEW_PADDING 8
#define CORE_ITEMVIEW_MARGIN_L 40
#define CORE_ITEMVIEW_DIVIDER_SIZE 3
#define CORE_ITEMVIEW_WIDTH 400
#define CORE_ITEMVIEW_HEIGHT (GUI_GetLineHeight() + CORE_ITEMVIEW_PADDING * 2)
#define CORE_ITEMVIEW_BORDER_SIZE 2
#define CORE_ITEMVIEW_NAME_COLOR 0xFFFFFFFF
#define CORE_ITEMVIEW_BG_COLOR 0x9F1F1F1F
#define CORE_ITEMVIEW_SEL_COLOR 0x9FC05012
#define CORE_ITEMVIEW_BORDER_COLOR 0xE0F0F0F0

CoreEntry nes_entries[] = {
    {"FCEUmm", "fceumm", NES_SHOR_NAME},
    {"Nestopia", "nestopia", NES_SHOR_NAME},
};

CoreEntry snes_entries[] = {
    {"Snes9x 2002", "snes9x2002", SNES_SHOR_NAME},
    {"Snes9x 2005", "snes9x2005", SNES_SHOR_NAME},
    {"Snes9x 2005 Plus", "snes9x2005_plus", SNES_SHOR_NAME},
};

CoreEntry md_entries[] = {
    {"Genesis Plus GX", "genesis_plus_gx", MD_SHOR_NAME},
    {"PicoDrive", "picodrive", MD_SHOR_NAME},
};

CoreEntry gba_entries[] = {
    {"gpSP", "gpsp", GBA_SHOR_NAME},
    {"VBA Next", "vba_next", GBA_SHOR_NAME},
};

CoreEntry gbc_entries[] = {
    {"Gambatte", "gambatte", GBC_SHOR_NAME},
};

CoreEntry pce_entries[] = {
    {"Mednafen PCE Fast", "mednafen_pce_fast", PCE_SHOR_NAME},
    {"Mednafen SuperGrafx", "mednafen_supergrafx", PCE_SHOR_NAME},
};

CoreEntry ps_entries[] = {
    {"PCSX ReARMed", "pcsx_rearmed", PS1_SHOR_NAME},
};

CoreEntry wsc_entries[] = {
    {"Mednafen Wswan", "mednafen_wswan", WSC_SHOR_NAME},
};

CoreEntry ngp_entries[] = {
    {"Mednafen NeoPop", "mednafen_ngp", NGP_SHOR_NAME},
};

CoreEntry arc_entries[] = {
    {"FBA Lite", "fba_lite", ARC_SHOR_NAME},
};

SoftwareEntry software_entries[] = {
    {NES_SHOR_NAME, NULL, nes_entries, sizeof(nes_entries) / sizeof(CoreEntry), &setting_config.nes_pos, {0}},
    {SNES_SHOR_NAME, NULL, snes_entries, sizeof(snes_entries) / sizeof(CoreEntry), &setting_config.snes_pos, {0}},
    {MD_SHOR_NAME, NULL, md_entries, sizeof(md_entries) / sizeof(CoreEntry), &setting_config.md_pos, {0}},
    {GBA_SHOR_NAME, NULL, gba_entries, sizeof(gba_entries) / sizeof(CoreEntry), &setting_config.gba_pos, {0}},
    {GBC_SHOR_NAME, NULL, gbc_entries, sizeof(gbc_entries) / sizeof(CoreEntry), &setting_config.gbc_pos, {0}},
    {PCE_SHOR_NAME, NULL, pce_entries, sizeof(pce_entries) / sizeof(CoreEntry), &setting_config.pce_pos, {0}},
    {PS1_SHOR_NAME, NULL, ps_entries, sizeof(ps_entries) / sizeof(CoreEntry), &setting_config.ps1_pos, {0}},
    {WSC_SHOR_NAME, NULL, wsc_entries, sizeof(wsc_entries) / sizeof(CoreEntry), &setting_config.wsc_pos, {0}},
    {NGP_SHOR_NAME, NULL, ngp_entries, sizeof(ngp_entries) / sizeof(CoreEntry), &setting_config.ngp_pos, {0}},
    {ARC_SHOR_NAME, NULL, arc_entries, sizeof(arc_entries) / sizeof(CoreEntry), &setting_config.arc_pos, {0}},
};
#define N_SOFTWARE_ENTRIES (sizeof(software_entries) / sizeof(SoftwareEntry))

static int core_entries_open = 0;

static float icon_focus_x = 0, icon_focus_y = 0;
static float icon_target_sx = 0, icon_target_sy = 0;
static float icon_current_sx = 0, icon_current_sy = 0;
static float icon_scroll_step = 0;

static uint32_t icon_tint_color = ICON_TINT_COLOR;

static uint32_t core_name_color = 0;
static uint32_t core_bg_color = 0;
static uint32_t core_sel_color = 0;
static uint32_t core_border_color = 0;

static WallpaperList wallpaper_list = {0};
static int wallpaper_sel = 0;

static void makeCorePath(char *path, char *name)
{
    snprintf(path, MAX_PATH_LENGTH, "app0:%s.self", name);
}

static void makeIconPath(char *path, char *name)
{
    snprintf(path, MAX_PATH_LENGTH, "%s/%s/icon0.png", CORE_DATA_DIR, name);
}

static uint32_t makeAlphaColor(uint32_t cur, uint32_t target, float alpha_step)
{
    int cur_alpha = COLOR_GET_ALPHA(cur);
    int target_alpha = COLOR_GET_ALPHA(target);

    if (alpha_step < 1)
        alpha_step = 1;

    if (cur_alpha < target_alpha)
    {
        cur_alpha += alpha_step;
        if (cur_alpha > target_alpha)
            cur_alpha = target_alpha;
    }
    else if (cur_alpha > target_alpha)
    {
        cur_alpha -= alpha_step;
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
    char assets_dir[MAX_PATH_LENGTH];
    makeCorePath(core_path, entry->core_name);
    snprintf(assets_dir, MAX_CONFIG_LINE_LENGTH, "%s/%s", CORE_DATA_DIR, entry->assets_name);

    int ret = BootLoadExecForCore(core_path, assets_dir);

    return ret;
}

static void refreshIconScroll()
{
    if (setting_config.software_pos == 0)
        icon_target_sx = icon_focus_x;
    else
        icon_target_sx = icon_focus_x - (setting_config.software_pos * (ICON_UNFOCUS_WIDTH + ICON_UNFOCUS_MARGIN) - ICON_UNFOCUS_MARGIN + ICON_FOCUS_MARGIN);

    if (icon_target_sx > icon_current_sx)
        icon_scroll_step = (float)(icon_target_sx - icon_current_sx) / (float)MAX_ICON_STEP_COUNT;
    else
        icon_scroll_step = (float)(icon_current_sx - icon_target_sx) / (float)MAX_ICON_STEP_COUNT;
}

static void initSoftwareEntriesLayout()
{
    int layout_x = 0, layout_y = 0;
    int layout_h = 0;
    GUI_GetActivityLayoutPosition(&layout_x, &layout_y);
    GUI_GetActivityAvailableSize(NULL, &layout_h);

    icon_focus_x = layout_x + ICON_UNFOCUS_WIDTH * 0.8f + ICON_FOCUS_MARGIN;
    icon_focus_y = layout_y + (layout_h - ICON_FOCUS_HIEGHT) / 5;

    icon_current_sx = icon_target_sx = icon_focus_x;
    icon_current_sy = icon_target_sy = icon_focus_y;

    // Set icon layout
    int x = icon_current_sx;
    int y = icon_current_sy;
    int i;
    for (i = 0; i < N_SOFTWARE_ENTRIES; i++)
    {
        IconLayout *layout = &software_entries[i].layout;
        layout->selview_border_color = 0;
        layout->selview_bg_color = 0;
        layout->shortname_color = 0;
        layout->x = x;
        layout->y = y;
        layout->w = ICON_UNFOCUS_WIDTH;
        layout->h = ICON_UNFOCUS_HIEGHT;
        x += (ICON_UNFOCUS_WIDTH + ICON_UNFOCUS_MARGIN);
    }

    refreshIconScroll();
}

static void updateSoftwareEntriesLayout()
{
    float old_current_sx = icon_current_sx;

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

    float real_scroll_step = icon_current_sx - old_current_sx;

    int target_w, target_h;
    float w_scale_step = (float)(ICON_FOCUS_WIDTH - ICON_UNFOCUS_WIDTH) / (float)MAX_ICON_STEP_COUNT;
    float h_scale_step = (float)(ICON_FOCUS_HIEGHT - ICON_UNFOCUS_HIEGHT) / (float)MAX_ICON_STEP_COUNT;

    float x = icon_current_sx;
    float y = icon_current_sy;
    float x_scroll_step2 = w_scale_step + (float)(ICON_FOCUS_MARGIN - ICON_UNFOCUS_MARGIN) / (float)MAX_ICON_STEP_COUNT;

    int i;
    for (i = 0; i < N_SOFTWARE_ENTRIES; i++)
    {
        IconLayout *layout = &software_entries[i].layout;

        layout->x += real_scroll_step;
        layout->y = y;

        if (i == setting_config.software_pos)
        {
            target_w = ICON_FOCUS_WIDTH;
            target_h = ICON_FOCUS_HIEGHT;

            if (layout->h == ICON_FOCUS_HIEGHT)
                layout->shortname_color = makeAlphaColor(layout->shortname_color, ICON_SHORT_NAME_COLOR, COLOR_GET_ALPHA(ICON_SHORT_NAME_COLOR) / (float)MAX_ICON_STEP_COUNT);
            layout->shortname_bg_color = makeAlphaColor(layout->shortname_bg_color, ICON_SHORT_NAME_BG_COLOR, COLOR_GET_ALPHA(ICON_SHORT_NAME_BG_COLOR) / (float)MAX_ICON_STEP_COUNT);
            layout->selview_bg_color = makeAlphaColor(layout->selview_bg_color, ICON_SELVIEW_BG_COLOR, COLOR_GET_ALPHA(ICON_SELVIEW_BG_COLOR) / (float)MAX_ICON_STEP_COUNT);
            if (!core_entries_open)
                layout->selview_border_color = makeAlphaColor(layout->selview_border_color, ICON_SELVIEW_BORDER_COLOR, COLOR_GET_ALPHA(ICON_SELVIEW_BORDER_COLOR) / (float)MAX_BORDER_COLOR_STEP_COUNT);
            else
                layout->selview_border_color = makeAlphaColor(layout->selview_border_color, COLOR_SET_ALPHA(ICON_SELVIEW_BORDER_COLOR, 0), COLOR_GET_ALPHA(ICON_SELVIEW_BORDER_COLOR) / (float)MAX_ICON_STEP_COUNT);
        }
        else
        {
            target_w = ICON_UNFOCUS_WIDTH;
            target_h = ICON_UNFOCUS_HIEGHT;

            layout->shortname_color = 0;
            layout->shortname_bg_color = makeAlphaColor(layout->shortname_bg_color, COLOR_SET_ALPHA(ICON_SHORT_NAME_BG_COLOR, 0), COLOR_GET_ALPHA(ICON_SHORT_NAME_BG_COLOR) / (float)MAX_ICON_STEP_COUNT);
            layout->selview_bg_color = makeAlphaColor(layout->selview_bg_color, COLOR_SET_ALPHA(ICON_SELVIEW_BG_COLOR, 0), COLOR_GET_ALPHA(ICON_SELVIEW_BG_COLOR) / (float)MAX_ICON_STEP_COUNT);
            layout->selview_border_color = makeAlphaColor(layout->selview_border_color, COLOR_SET_ALPHA(ICON_SELVIEW_BORDER_COLOR, 0), COLOR_GET_ALPHA(ICON_SELVIEW_BORDER_COLOR) / (float)MAX_BORDER_COLOR_STEP_COUNT);
        }

        // Scale icon width
        if (layout->w < target_w)
        {
            layout->w += w_scale_step;
            if (layout->w > target_w)
                layout->w = target_w;
        }
        else if (layout->w > target_w)
        {
            layout->w -= w_scale_step;
            if (layout->w < target_w)
                layout->w = target_w;
        }

        // Scale icon height
        if (layout->h < target_h)
        {
            layout->h += h_scale_step;
            if (layout->h > target_h)
                layout->h = target_h;
        }
        else if (layout->h > target_h)
        {
            layout->h -= h_scale_step;
            if (layout->h < target_h)
                layout->h = target_h;
        }

        // Scale icon x
        if (layout->x < x)
        {
            layout->x += x_scroll_step2;
            if (layout->x > x)
                layout->x = x;
        }
        else if (layout->x > x)
        {
            layout->x -= x_scroll_step2;
            if (layout->x < x)
                layout->x = x;
        }

        int icon_w = ICON_UNFOCUS_WIDTH;
        int margin_r = ICON_UNFOCUS_MARGIN;
        if (i == setting_config.software_pos || i == setting_config.software_pos - 1)
        {
            if (i == setting_config.software_pos)
                icon_w = ICON_FOCUS_WIDTH;
            margin_r = ICON_FOCUS_MARGIN;
        }
        x += (icon_w + margin_r);
    }

    if (core_entries_open)
    {
        // 图标逐渐不见
        icon_tint_color = makeAlphaColor(icon_tint_color, COLOR_SET_ALPHA(ICON_TINT_COLOR, 0), COLOR_GET_ALPHA(ICON_TINT_COLOR) / (float)MAX_ICON_STEP_COUNT);
    }
    else
    {
        // 图标逐渐可见
        icon_tint_color = makeAlphaColor(icon_tint_color, ICON_TINT_COLOR, COLOR_GET_ALPHA(ICON_TINT_COLOR) / (float)MAX_ICON_STEP_COUNT);
    }
}

static void moveSoftwareEntriesPos(int type)
{
    uint32_t pos = setting_config.software_pos;
    uint32_t max = N_SOFTWARE_ENTRIES - 1;

    if (type == TYPE_MOVE_LEFT)
    {
        if (pos > 0)
            --pos;
    }
    else if (type == TYPE_MOVE_RIGHT)
    {
        if (pos < N_SOFTWARE_ENTRIES - 1)
            ++pos;
    }
    if (pos > max)
        pos = max;

    if (pos != setting_config.software_pos)
    {
        setting_config.software_pos = pos;
        refreshIconScroll();
    }
}

static void updateCoreEntriesLayout()
{
    if (core_entries_open)
    {
        // 核心列表文字和背景颜色渐变
        core_name_color = makeAlphaColor(core_name_color, CORE_ITEMVIEW_NAME_COLOR, COLOR_GET_ALPHA(CORE_ITEMVIEW_NAME_COLOR) / (float)MAX_ICON_STEP_COUNT);
        core_bg_color = makeAlphaColor(core_bg_color, CORE_ITEMVIEW_BG_COLOR, COLOR_GET_ALPHA(CORE_ITEMVIEW_BG_COLOR) / (float)MAX_ICON_STEP_COUNT);
        core_sel_color = makeAlphaColor(core_sel_color, CORE_ITEMVIEW_SEL_COLOR, COLOR_GET_ALPHA(CORE_ITEMVIEW_SEL_COLOR) / (float)MAX_ICON_STEP_COUNT);
        core_border_color = makeAlphaColor(core_border_color, CORE_ITEMVIEW_BORDER_COLOR, COLOR_GET_ALPHA(CORE_ITEMVIEW_BORDER_COLOR) / (float)MAX_ICON_STEP_COUNT);
    }
    else
    { // 核心列表文字和背景颜色渐变
        core_name_color = makeAlphaColor(core_name_color, COLOR_SET_ALPHA(CORE_ITEMVIEW_NAME_COLOR, 0), COLOR_GET_ALPHA(CORE_ITEMVIEW_NAME_COLOR) / (float)MAX_ICON_STEP_COUNT);
        core_bg_color = makeAlphaColor(core_bg_color, COLOR_SET_ALPHA(CORE_ITEMVIEW_BG_COLOR, 0), COLOR_GET_ALPHA(CORE_ITEMVIEW_BG_COLOR) / (float)MAX_ICON_STEP_COUNT);
        core_sel_color = makeAlphaColor(core_sel_color, COLOR_SET_ALPHA(CORE_ITEMVIEW_SEL_COLOR, 0), COLOR_GET_ALPHA(CORE_ITEMVIEW_SEL_COLOR) / (float)MAX_ICON_STEP_COUNT);
        core_border_color = makeAlphaColor(core_border_color, COLOR_SET_ALPHA(CORE_ITEMVIEW_BORDER_COLOR, 0), COLOR_GET_ALPHA(CORE_ITEMVIEW_BORDER_COLOR) / (float)MAX_ICON_STEP_COUNT);
    }
}

static void moveCoreEntriesPos(int type)
{
    uint32_t pos = *software_entries[setting_config.software_pos].entries_pos;
    uint32_t max = software_entries[setting_config.software_pos].n_entries - 1;

    if (type == TYPE_MOVE_UP)
    {
        if (pos > 0)
            --pos;
    }
    else if (type == TYPE_MOVE_DOWN)
    {
        if (pos < max)
            ++pos;
    }
    if (pos > max)
        pos = max;

    *software_entries[setting_config.software_pos].entries_pos = pos;
}

static void openCoreEntries()
{
    core_entries_open = 1;
    moveCoreEntriesPos(TYPE_MOVE_NONE);
    refreshIconScroll();

    int x_scroll_size = ICON_UNFOCUS_WIDTH * 0.5f;

    icon_target_sx -= x_scroll_size;
    if (icon_target_sx > icon_current_sx)
        icon_scroll_step = (float)(icon_target_sx - icon_current_sx) / (float)MAX_ICON_STEP_COUNT;
    else
        icon_scroll_step = (float)(icon_current_sx - icon_target_sx) / (float)MAX_ICON_STEP_COUNT;

    core_name_color = 0;
    core_bg_color = 0;
    core_sel_color = 0;
    core_border_color = 0;
}

static void closeCoreEntries()
{
    core_entries_open = 0;
    moveCoreEntriesPos(TYPE_MOVE_NONE);
    refreshIconScroll();
}

static int drawCoreEntries()
{
    if (!core_entries_open && !COLOR_GET_ALPHA(core_name_color))
        return 0;

    updateCoreEntriesLayout();

    SoftwareEntry *software_entry = &software_entries[setting_config.software_pos];
    CoreEntry *core_entries = software_entry->entries;

    int item_x = software_entry->layout.x + software_entry->layout.w + CORE_ITEMVIEW_MARGIN_L;
    int item_y = software_entry->layout.y;
    int item_w = CORE_ITEMVIEW_WIDTH;
    int item_h = CORE_ITEMVIEW_HEIGHT;

    int i;
    for (i = 0; i < software_entry->n_entries; i++)
    {
        vita2d_draw_rectangle(item_x, item_y, item_w, item_h, core_bg_color);

        if (i == *software_entry->entries_pos)
        {
            vita2d_draw_rectangle(item_x, item_y, item_w, item_h, core_sel_color);

            int border_x = item_x - CORE_ITEMVIEW_BORDER_SIZE;
            int border_y = item_y - CORE_ITEMVIEW_BORDER_SIZE;
            int border_w = item_w + CORE_ITEMVIEW_BORDER_SIZE * 2;
            int border_h = item_h + CORE_ITEMVIEW_BORDER_SIZE * 2;
            vita2d_draw_empty_rectangle(border_x, border_y, border_w, border_h, CORE_ITEMVIEW_BORDER_SIZE, core_border_color);
        }

        GUI_DrawText(item_x + CORE_ITEMVIEW_PADDING, item_y + CORE_ITEMVIEW_PADDING, core_name_color, core_entries[i].desc);

        item_y += (item_h + CORE_ITEMVIEW_DIVIDER_SIZE);
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
    else if (pressed_pad[PAD_CANCEL])
    {
        closeCoreEntries();
    }
    else if (pressed_pad[PAD_ENTER])
    {
        SoftwareEntry *software_entry = &software_entries[setting_config.software_pos];
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
    int layout_sx = 0, layout_sy = 0;
    int layout_w = 0, layout_h = 0;
    int layout_dx;
    float x_scale, y_scale;
    uint32_t tint_color;

    GUI_GetActivityLayoutPosition(&layout_sx, &layout_sy);
    GUI_GetActivityAvailableSize(&layout_w, &layout_h);
    layout_dx = layout_sx + layout_w;

    int i;
    for (i = 0; i < N_SOFTWARE_ENTRIES; i++)
    {
        entry = &software_entries[i];
        layout = &entry->layout;

        if (layout->x + layout->w < layout_sx)
            continue;
        if (layout->x >= layout_dx)
            break;

        tint_color = icon_tint_color;
        if (i == setting_config.software_pos)
            tint_color = ICON_TINT_COLOR;

        // Draw selview bg
        int selview_x = layout->x;
        int selview_y = layout->y;
        int selview_w = layout->w;
        int selview_h = layout->h + ICON_SHORT_NAME_GET_HEIGHT(layout->h);
        if (COLOR_GET_ALPHA(layout->selview_bg_color))
            vita2d_draw_rectangle(selview_x, selview_y, selview_w, selview_h, layout->selview_bg_color);

        if (COLOR_GET_ALPHA(tint_color))
        {
            if (entry->icon)
            {
                x_scale = (float)layout->w / (float)vita2d_texture_get_width(entry->icon);
                y_scale = (float)layout->h / (float)vita2d_texture_get_height(entry->icon);
                vita2d_draw_texture_tint_scale(entry->icon, layout->x, layout->y, x_scale, y_scale, tint_color);
            }
            else
            {
                vita2d_draw_rectangle(layout->x, layout->y, layout->w, layout->h, ICON_EMPTY_COLOR & tint_color);
            }
        }

        // Draw short name
        int shortname_view_x = selview_x;
        int shortname_view_y = selview_y + layout->h;
        int shortname_view_w = selview_w;
        int shortname_view_h = selview_h - layout->h;
        int shortname_text_x = shortname_view_x + (shortname_view_w - GUI_GetTextWidth(entry->short_name)) / 2;
        int shortname_text_y = shortname_view_y + (shortname_view_h - GUI_GetLineHeight()) / 2;
        if (COLOR_GET_ALPHA(layout->shortname_bg_color))
            vita2d_draw_rectangle(shortname_view_x, shortname_view_y, shortname_view_w, shortname_view_h, layout->shortname_bg_color);
        if (COLOR_GET_ALPHA(layout->shortname_color))
            GUI_DrawText(shortname_text_x, shortname_text_y, layout->shortname_color, entry->short_name);

        // Draw selview border
        int selview_border_x = selview_x - 1 - ICON_SELVIEW_BORDER_SIZE;
        int selview_border_y = selview_y - 1 - ICON_SELVIEW_BORDER_SIZE;
        int selview_border_w = selview_w + 2 + ICON_SELVIEW_BORDER_SIZE * 2;
        int selview_border_h = selview_h + 2 + ICON_SELVIEW_BORDER_SIZE * 2;
        if (COLOR_GET_ALPHA(layout->selview_border_color))
            vita2d_draw_empty_rectangle(selview_border_x, selview_border_y, selview_border_w, selview_border_h, ICON_SELVIEW_BORDER_SIZE, layout->selview_border_color);
    }

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
    else if (pressed_pad[PAD_ENTER])
    {
        if (icon_current_sx == icon_target_sx)
            openCoreEntries();
    }

    return 0;
}

static int ctrlCommon()
{
    if (released_pad[PAD_L1])
    {
        if (wallpaper_sel > 0)
            --wallpaper_sel;
        else
            wallpaper_sel = wallpaper_list.length - 1;
    }
    else if (released_pad[PAD_R1])
    {
        if (wallpaper_sel < wallpaper_list.length - 1)
            ++wallpaper_sel;
        else
            wallpaper_sel = 0;
    }

    return 0;
}

int BrowserDraw()
{
    ctrlCommon();
    drawCoreEntries();
    drawSoftwareEntries();

    return 0;
}

int BrowserCtrl()
{
    if (core_entries_open)
        ctrlCoreEntries();
    else
        ctrlSoftwareEntries();

    return 0;
}

static int iconsThreadEntry(SceSize args, void *argp)
{
    vita2d_texture *texture;
    char path[MAX_PATH_LENGTH];

    int i;
    for (i = 0; i < N_SOFTWARE_ENTRIES; i++)
    {
        makeIconPath(path, software_entries[i].short_name);
        texture = vita2d_load_PNG_file(path);
        vita2d_texture_set_filters(texture, SCE_GXM_TEXTURE_FILTER_LINEAR, SCE_GXM_TEXTURE_FILTER_LINEAR);
        setIconTexture(&software_entries[i], texture);
    }

    sceKernelExitDeleteThread(0);
    return 0;
}

static int wallpaperThreadEntry(SceSize args, void *argp)
{
    int old_wallpaper_sel = -1;
    WallpaperEntry *entry = NULL;
    vita2d_texture *textures[2] = {0};
    int texture_idx = 0, next_texture_idx = 0;
    char path[MAX_PATH_LENGTH];

    // 读取wallpapers列表
    strcpy(wallpaper_list.path, WALLPAPER_DIR);
    WallpaperListGetEntries(&wallpaper_list);

    if (wallpaper_list.length <= 1)
    {
        entry = wallpaper_list.head;
        if (!entry)
            goto EXIT;
        snprintf(path, MAX_PATH_LENGTH, "%s/%s", wallpaper_list.path, entry->name);

        textures[texture_idx] = vita2d_load_PNG_file(path);
        if (textures[texture_idx])
        {
            GUI_LockDrawMutex();
            GUI_SetWallpaperTexture(textures[texture_idx]);
            GUI_UnlockDrawMutex();
            WriteFile(WALLPAPER_SAVEFILE_PATH, entry->name, strlen(entry->name) + 1);
        }
        goto EXIT;
    }

    if (ReadFile(WALLPAPER_SAVEFILE_PATH, path, MAX_PATH_LENGTH) > 0)
        wallpaper_sel = WallpaperListGetNumByName(&wallpaper_list, path);
    if (wallpaper_sel < 0)
        wallpaper_sel = 0;

    while (AppIsRunning())
    {
        if (old_wallpaper_sel == wallpaper_sel)
        {
            sceKernelDelayThread(1000);
            continue;
        }

        old_wallpaper_sel = wallpaper_sel;
        entry = WallpaperListFindEntryByNum(&wallpaper_list, wallpaper_sel);
        if (!entry)
            continue;
        snprintf(path, MAX_PATH_LENGTH, "%s/%s", wallpaper_list.path, entry->name);

        next_texture_idx = (texture_idx + 1) % 2;
        textures[next_texture_idx] = vita2d_load_PNG_file(path);
        if (textures[next_texture_idx])
        {
            GUI_LockDrawMutex();
            if (textures[texture_idx])
            {
                vita2d_wait_rendering_done();
                vita2d_free_texture(textures[texture_idx]);
                textures[texture_idx] = NULL;
            }
            GUI_SetWallpaperTexture(textures[next_texture_idx]);
            GUI_UnlockDrawMutex();
            texture_idx = next_texture_idx;
            WriteFile(WALLPAPER_SAVEFILE_PATH, entry->name, strlen(entry->name) + 1);
        }
    }

    GUI_LockDrawMutex();
    GUI_SetWallpaperTexture(NULL);
    GUI_UnlockDrawMutex();

    int i;
    for (i = 0; i < 2; i++)
    {
        if (textures[i])
        {
            vita2d_wait_rendering_done();
            vita2d_free_texture(textures[texture_idx]);
        }
    }

EXIT:
    WallpaperListEmpty(&wallpaper_list);
    sceKernelExitDeleteThread(0);
    return 0;
}

static void startIconsThread()
{
    SceUID thid = sceKernelCreateThread("icons_thread", iconsThreadEntry, 0x10000100, 0x10000, 0, 0, NULL);
    if (thid >= 0)
        sceKernelStartThread(thid, 0, NULL);
}

static void startWallpaperThread()
{
    SceUID thid = sceKernelCreateThread("wallpaper_thread", wallpaperThreadEntry, 0x10000100, 0x10000, 0, 0, NULL);
    if (thid >= 0)
        sceKernelStartThread(thid, 0, NULL);
}

int BrowserInit()
{
    LoadConfig();
    startIconsThread();
    startWallpaperThread();
    initSoftwareEntriesLayout();

    return 0;
}

int BrowserDeinit()
{
    int i;
    for (i = 0; i < N_SOFTWARE_ENTRIES; i++)
    {
        if (software_entries[i].icon)
        {
            vita2d_wait_rendering_done();
            vita2d_free_texture(software_entries[i].icon);
            software_entries[i].icon = NULL;
        }
    }

    return 0;
}
