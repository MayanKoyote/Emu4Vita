#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <psp2/kernel/threadmgr.h>

#include <vita2d.h>
#include <vita2d_ext.h>

#include "list/linked_list.h"
#include "gui_lib.h"
#include "file.h"
#include "config.h"
#include "utils.h"

extern float _vita2d_ortho_matrix[4 * 4];

static SceKernelLwMutexWork gui_draw_mutex;
static LinkedList *gui_clip_list = NULL;

void GUI_DrawEmptyRectangle(float x, float y, float w, float h, float line_size, unsigned int color)
{
    float sx = x;
    float sy = y;
    float dx = x + w;
    float dy = y + h;
    vita2d_draw_rectangle(sx, sy, line_size, h - line_size, color);
    vita2d_draw_rectangle(sx + line_size, sy, w - line_size, line_size, color);
    vita2d_draw_rectangle(dx - line_size, sy + line_size, line_size, h - line_size, color);
    vita2d_draw_rectangle(sx, dy - line_size, w - line_size, line_size, color);
}

void GUI_DestroyTexture(GUI_Texture *texture)
{
    GUI_WaitRenderingDone();
    vita2d_free_texture(texture);
}

void GUI_DrawTextureShaderPartScalRotate(const GUI_Texture *texture, const GUI_Shader *shader, float x, float y,
                                         float tex_x, float tex_y, float tex_w, float tex_h, float x_scale, float y_scale, float rad)
{
    vita2d_set_shader(shader);

    float *texture_size = (float *)vita2d_pool_memalign(2 * sizeof(float), sizeof(float));
    texture_size[0] = (float)vita2d_texture_get_width(texture);
    texture_size[1] = (float)vita2d_texture_get_height(texture);

    float *output_size = (float *)vita2d_pool_memalign(2 * sizeof(float), sizeof(float));
    output_size[0] = (float)vita2d_texture_get_width(texture) * x_scale;
    output_size[1] = (float)vita2d_texture_get_height(texture) * y_scale;

    vita2d_set_vertex_uniform(shader, "IN.texture_size", texture_size, 2);
    vita2d_set_vertex_uniform(shader, "IN.video_size", texture_size, 2);
    vita2d_set_vertex_uniform(shader, "IN.output_size", output_size, 2);

    vita2d_set_fragment_uniform(shader, "IN.texture_size", texture_size, 2);
    vita2d_set_fragment_uniform(shader, "IN.video_size", texture_size, 2);
    vita2d_set_fragment_uniform(shader, "IN.output_size", output_size, 2);

    vita2d_set_wvp_uniform(shader, _vita2d_ortho_matrix);

    vita2d_draw_texture_part_scale_rotate_generic(texture, x, y, tex_x, tex_y, tex_w, tex_h, x_scale, y_scale, rad);
}

void GUI_LockDraw()
{
    sceKernelLockLwMutex(&gui_draw_mutex, 1, NULL);
}

void GUI_UnlockDraw()
{
    sceKernelUnlockLwMutex(&gui_draw_mutex, 1);
}

void GUI_StartDrawing(GUI_Texture *texture)
{
    vita2d_pool_reset();
    vita2d_start_drawing_advanced(texture, 0);
    vita2d_clear_screen();
}

void GUI_EndDrawing()
{
    vita2d_end_drawing();
    // vita2d_common_dialog_update();
    vita2d_swap_buffers();
}

int GUI_SetClipping(int x, int y, int w, int h)
{
    if (!gui_clip_list)
    {
        gui_clip_list = NewLinkedList();
        if (!gui_clip_list)
            return -1;
        LinkedListSetFreeEntryDataCallback(gui_clip_list, free);
    }

    GUI_Rect *cur_rect = (GUI_Rect *)malloc(sizeof(GUI_Rect));
    if (!cur_rect)
        return -1;

    int sx = x;
    int sy = y;
    int dx = x + w;
    int dy = y + h;

    LinkedListEntry *prev_entry = LinkedListTail(gui_clip_list);
    GUI_Rect *prev_rect = (GUI_Rect *)LinkedListGetEntryData(prev_entry);

    if (prev_rect)
    {
        if (sx < prev_rect->x)
            sx = prev_rect->x;
        if (sy < prev_rect->y)
            sy = prev_rect->y;
        if (dx > prev_rect->x + prev_rect->w)
            dx = prev_rect->x + prev_rect->w;
        if (dy > prev_rect->y + prev_rect->h)
            dy = prev_rect->y + prev_rect->h;
    }
    else
    {
        vita2d_enable_clipping();
    }

    vita2d_set_clip_rectangle(sx, sy, dx, dy);

    cur_rect->x = sx;
    cur_rect->y = sy;
    cur_rect->w = dx - sx;
    cur_rect->h = dy - sy;
    LinkedListAdd(gui_clip_list, cur_rect);

    return 0;
}

int GUI_UnsetClipping()
{
    if (!gui_clip_list)
        return -1;

    LinkedListEntry *cur_entry = LinkedListTail(gui_clip_list);
    if (cur_entry)
    {
        LinkedListEntry *prev_entry = LinkedListPrev(cur_entry);
        GUI_Rect *prev_rect = (GUI_Rect *)LinkedListGetEntryData(prev_entry);

        if (prev_rect)
            vita2d_set_clip_rectangle(prev_rect->x, prev_rect->y, prev_rect->x + prev_rect->w, prev_rect->y + prev_rect->h);
        else
            vita2d_disable_clipping();

        LinkedListRemove(gui_clip_list, cur_entry);
    }

    return 0;
}

int GUI_InitLib()
{
    sceKernelCreateLwMutex(&gui_draw_mutex, "gui_draw_mutex", 2, 0, NULL);
    vita2d_init();
    vita2d_ext_init(vita2d_get_context(), vita2d_get_shader_patcher());
    vita2d_set_vblank_wait(1);
    return 0;
}

int GUI_DeinitLib()
{
    vita2d_ext_fini();
    vita2d_fini();
    sceKernelDeleteLwMutex(&gui_draw_mutex);
    return 0;
}