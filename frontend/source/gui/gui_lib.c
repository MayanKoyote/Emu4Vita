#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <vita2d.h>
#include <vita2d_ext.h>

#include "list/linked_list.h"
#include "gui_lib.h"
#include "file.h"
#include "config.h"
#include "utils.h"

extern float _vita2d_ortho_matrix[4 * 4];

static LinkedList *gui_clip_list = NULL;

void GUI_DrawEmptyRectangle(int x, int y, int w, int h, int size, unsigned int color)
{
    int x2 = x + w;
    int y2 = y + h;
    vita2d_draw_rectangle(x, y, size, h - size, color);
    vita2d_draw_rectangle(x + size, y, w - size, size, color);
    vita2d_draw_rectangle(x2 - size, y + size, size, h - size, color);
    vita2d_draw_rectangle(x, y2 - size, w - size, size, color);
}

void GUI_DestroyTexture(GUI_Texture *texture)
{
    GUI_WaitRenderingDone();
    vita2d_free_texture(texture);
}

void GUI_DrawTextureShaderPartScalRotate(const GUI_Texture *texture, const GUI_Shader *shader, int x, int y,
                                         int tex_x, int tex_y, int tex_w, int tex_h, float x_scale, float y_scale, float rad)
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
}

void GUI_RenderPresent()
{
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

    int x2 = x + w;
    int y2 = y + h;

    LinkedListEntry *prev_entry = LinkedListTail(gui_clip_list);
    GUI_Rect *prev_rect = (GUI_Rect *)LinkedListGetEntryData(prev_entry);

    if (prev_rect)
    {
        if (x < prev_rect->x)
            x = prev_rect->x;
        if (y < prev_rect->y)
            y = prev_rect->y;
        if (x2 > prev_rect->x + prev_rect->w)
            x2 = prev_rect->x + prev_rect->w;
        if (y2 > prev_rect->y + prev_rect->h)
            y2 = prev_rect->y + prev_rect->h;
    }
    else
    {
        vita2d_enable_clipping();
    }

    vita2d_set_clip_rectangle(x, y, x2, y2);

    cur_rect->x = x;
    cur_rect->y = y;
    cur_rect->w = x2 - x;
    cur_rect->h = y2 - y;
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
    vita2d_init();
    vita2d_ext_init(vita2d_get_context(), vita2d_get_shader_patcher());
    vita2d_set_vblank_wait(1);
    return 0;
}

int GUI_DeinitLib()
{
    vita2d_ext_fini();
    vita2d_fini();
    return 0;
}