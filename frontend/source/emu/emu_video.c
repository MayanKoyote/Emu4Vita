#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <psp2/kernel/processmgr.h>

#include "setting/setting.h"
#include "emu/emu.h"
#include "utils.h"
#include "config.h"
#include "file.h"
#include "lang.h"

#define MICROS_PER_SECOND 1000000llu
#define SHOW_PLAYER_DURATION_MICROS (MICROS_PER_SECOND * 2)

#define MAX_TEXTURE_BUFS 3

static int video_okay = 0, video_pause = 1;
static int video_display_need_update = 1;

static SceUID video_semaid = -1;
static int video_texture_index = 0;
static GUI_Texture *video_texture_bufs[MAX_TEXTURE_BUFS] = {0};
static GUI_Texture *video_texture = NULL;
static GUI_Texture *overlay_texture = NULL;
static GUI_Shader *video_shader = NULL;
static int video_overlay_select = -1;

static unsigned int video_width = 0, video_height = 0;
static float video_x = 0.0f, video_y = 0.0f;
static float video_x_scale = 1.0f, video_y_scale = 1.0f;
static float video_rotate_rad = 0.0f;

static unsigned int video_frames = 0;
static float video_fps = 0.0f;

static uint64_t micros_per_frame = 0;
static uint64_t last_frame_micros = 0;

static uint64_t last_fps_micros = 0;
static uint64_t show_player_micros = 0;

void Emu_PauseVideo()
{
    video_pause = 1;
    GUI_SetVblankWait(1);
    Emu_SignalVideoSema(); // 防止卡死
}

void Emu_ResumeVideo()
{
    video_pause = 0;
    GUI_SetVblankWait(0);
}

void Emu_RequestUpdateVideoDisplay()
{
    video_display_need_update = 1;
}

void Emu_ShowControllerPortToast()
{
    show_player_micros = sceKernelGetProcessTimeWide() + SHOW_PLAYER_DURATION_MICROS;
}

GUI_Texture *Emu_GetCurrentVideoTexture()
{
    return video_texture;
}

GUI_Texture *Emu_GetNextVideoTexture()
{
    int next_texture_index = (video_texture_index + 1) % MAX_TEXTURE_BUFS;
    GUI_Texture *next_texture = video_texture_bufs[next_texture_index];
    return next_texture;
}

static int converRotateCWToCCW(int rotate_cw)
{
    int rotate_ccw = rotate_cw;
    if (rotate_cw == 1)
        rotate_ccw = 3;
    else if (rotate_cw == 3)
        rotate_ccw = 1;
    return rotate_ccw;
}

int Emu_GetVideoDisplayRotate()
{
    OverlayListEntryData *overlay_data = NULL;
    if (graphics_overlay_list && graphics_config.overlay_select > 0)
    {
        LinkedListEntry *entry = LinkedListFindByNum(graphics_overlay_list, graphics_config.overlay_select - 1);
        overlay_data = (OverlayListEntryData *)LinkedListGetEntryData(entry);
    }

    if (overlay_data && overlay_data->viewport_rotate)
        return converRotateCWToCCW(*(overlay_data->viewport_rotate));
    else if (graphics_config.display_rotate == TYPE_DISPLAY_ROTATE_DEFAULT)
        return core_display_rotate;
    else
        return converRotateCWToCCW(graphics_config.display_rotate);
}

void Emu_GetVideoBaseWH(uint32_t *width, uint32_t *height)
{
    if (!width || !height)
        return;

    OverlayListEntryData *overlay_data = NULL;
    uint32_t base_width = core_system_av_info.geometry.base_width;
    uint32_t base_height = core_system_av_info.geometry.base_height;
    float aspect_ratio = 0;

    if (graphics_overlay_list && graphics_config.overlay_select > 0)
    {
        LinkedListEntry *entry = LinkedListFindByNum(graphics_overlay_list, graphics_config.overlay_select - 1);
        overlay_data = (OverlayListEntryData *)LinkedListGetEntryData(entry);
    }

    if (overlay_data && overlay_data->viewport_width && overlay_data->viewport_height)
    {
        int rotate = Emu_GetVideoDisplayRotate();
        if (rotate == TYPE_DISPLAY_ROTATE_CW_90 || rotate == TYPE_DISPLAY_ROTATE_CW_270)
            aspect_ratio = (float)*(overlay_data->viewport_height) / (float)*(overlay_data->viewport_width);
        else
            aspect_ratio = (float)*(overlay_data->viewport_width) / (float)*(overlay_data->viewport_height);
    }
    else if (graphics_config.aspect_ratio == TYPE_DISPLAY_RATIO_BY_GAME_RESOLUTION)
    {
        *width = base_width;
        *height = base_height;
        return;
    }
    else if (graphics_config.aspect_ratio == TYPE_DISPLAY_RATIO_DEFAULT)
    {
        aspect_ratio = core_system_av_info.geometry.aspect_ratio;
    }
    else if (graphics_config.aspect_ratio == TYPE_DISPLAY_RATIO_BY_DEVICE_SCREEN)
    {
        aspect_ratio = (float)GUI_SCREEN_WIDTH / (float)GUI_SCREEN_HEIGHT;
    }
    else if (graphics_config.aspect_ratio == TYPE_DISPLAY_RATIO_8_7)
    {
        aspect_ratio = 8.f / 7.f;
    }
    else if (graphics_config.aspect_ratio == TYPE_DISPLAY_RATIO_4_3)
    {
        aspect_ratio = 4.f / 3.f;
    }
    else if (graphics_config.aspect_ratio == TYPE_DISPLAY_RATIO_3_2)
    {
        aspect_ratio = 3.f / 2.f;
    }
    else if (graphics_config.aspect_ratio == TYPE_DISPLAY_RATIO_16_9)
    {
        aspect_ratio = 16.f / 9.f;
    }

    if (aspect_ratio == 0)
        aspect_ratio = (float)base_width / (float)base_height;

    uint32_t new_width = base_width;
    uint32_t new_height = new_width / aspect_ratio;
    if (new_height < base_height)
    {
        new_height = base_height;
        new_width = new_height * aspect_ratio;
    }
    *width = new_width;
    *height = new_height;
}

void Emu_GetVideoDisplayWH(uint32_t *width, uint32_t *height)
{
    OverlayListEntryData *overlay_data = NULL;
    if (graphics_overlay_list && graphics_config.overlay_select > 0)
    {
        LinkedListEntry *entry = LinkedListFindByNum(graphics_overlay_list, graphics_config.overlay_select - 1);
        overlay_data = (OverlayListEntryData *)LinkedListGetEntryData(entry);
    }

    if (overlay_data && overlay_data->viewport_width && overlay_data->viewport_height)
    {
        *width = *(overlay_data->viewport_width);
        *height = *(overlay_data->viewport_height);
        return;
    }

    uint32_t base_width = 0, base_height = 0;
    uint32_t new_width = 0, new_height = 0;
    float aspect_ratio;

    Emu_GetVideoBaseWH(&base_width, &base_height);

    int rotate = Emu_GetVideoDisplayRotate();
    if (rotate == TYPE_DISPLAY_ROTATE_CW_90 || rotate == TYPE_DISPLAY_ROTATE_CW_270)
    {
        uint32_t temp_width = base_width;
        base_width = base_height;
        base_height = temp_width;
    }

    aspect_ratio = (float)base_width / (float)base_height;

    if (graphics_config.display_size == TYPE_DISPLAY_SIZE_2X)
    { // 2倍大小
        new_width = base_width * 2;
        new_height = base_height * 2;
    }
    else if (graphics_config.display_size == TYPE_DISPLAY_SIZE_3X)
    { // 3倍大小
        new_width = base_width * 3;
        new_height = base_height * 3;
    }
    else if (graphics_config.display_size == TYPE_DISPLAY_SIZE_FULL)
    { // 铺满屏幕
        new_height = GUI_SCREEN_HEIGHT;
        new_width = new_height * aspect_ratio;
    }
    else
    { // 1倍大小
        new_width = base_width;
        new_height = base_height;
    }

    // 检测越界
    if (new_width > GUI_SCREEN_WIDTH)
    {
        new_width = GUI_SCREEN_WIDTH;
        new_height = new_width / aspect_ratio;
    }
    if (new_height > GUI_SCREEN_HEIGHT)
    {
        new_height = GUI_SCREEN_HEIGHT;
        new_width = new_height * aspect_ratio;
    }

    *width = new_width;
    *height = new_height;
}

uint32_t *Emu_GetVideoScreenshotData(uint32_t *width, uint32_t *height, uint64_t *size, int rotate, int use_shader)
{
    if (!video_okay || !video_texture)
        return NULL;

    GUI_Texture *rendert_tex = NULL;
    uint32_t *conver_data = NULL;
    uint32_t conver_width = *width;
    uint32_t conver_height = *height;
    float aspect_ratio = (float)*width / (float)*height;

    // Fix above
    if (conver_width > GUI_SCREEN_WIDTH)
    {
        conver_width = GUI_SCREEN_WIDTH;
        conver_height = conver_width / aspect_ratio;
    }
    if (conver_height > GUI_SCREEN_HEIGHT)
    {
        conver_height = GUI_SCREEN_HEIGHT;
        conver_width = conver_height * aspect_ratio;
    }
    conver_width = (conver_width + 7) & ~7;
    if (conver_width != *width)
        conver_height = conver_width / aspect_ratio;

    // Get rotate rad
    float radian = 270 * 0.0174532925f;
    float rotate_rad = rotate * radian;

    // Get x y scale
    float x_scale, y_scale;
    if (rotate == TYPE_DISPLAY_ROTATE_CW_90 || rotate == TYPE_DISPLAY_ROTATE_CW_270)
    {
        x_scale = (float)conver_height / (float)video_width;
        y_scale = (float)conver_width / (float)video_height;
    }
    else
    {
        x_scale = (float)conver_width / (float)video_width;
        y_scale = (float)conver_height / (float)video_height;
    }

    // Use gpu to conver image
    rendert_tex = GUI_CreateTextureRendertarget(GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT, SCEEENSHOT_PIXEL_FORMAT);
    if (!rendert_tex)
    {
        APP_LOG("[VIDEO] Emu_GetVideoScreenshotData failed: cannot creat the rendertarget texture!\n");
        goto END;
    }

    GUI_LockDrawMutex();
    GUI_StartDrawing(rendert_tex);
    if (video_shader && use_shader)
        GUI_DrawTextureShaderPartScalRotate(video_texture, video_shader, conver_width / 2, conver_height / 2, 0, 0,
                                            video_width, video_height, x_scale, y_scale, rotate_rad);
    else
        GUI_DrawTexturePartScaleRotate(video_texture, conver_width / 2, conver_height / 2, 0, 0,
                                       video_width, video_height, x_scale, y_scale, rotate_rad);
    GUI_EndDrawing();
    GUI_WaitRenderingDone();
    // sceDisplayWaitVblankStart();
    GUI_UnlockDrawMutex();

    // Alloc and copy screenshot data
    uint64_t conver_size = conver_width * conver_height * 4;
    conver_data = (uint32_t *)malloc(conver_size);
    if (!conver_data)
        goto END;
    uint8_t *conver_datap = (uint8_t *)conver_data;
    uint32_t conver_stride = conver_width * 4;
    uint8_t *tex_datap = (uint8_t *)GUI_GetTextureDatap(rendert_tex);
    uint32_t tex_stride = GUI_GetTextureStride(rendert_tex);

    int i;
    for (i = 0; i < conver_height; i++)
    {
        memcpy(conver_datap, tex_datap, conver_stride);
        tex_datap += tex_stride;
        conver_datap += conver_stride;
    }

    *width = conver_width;
    *height = conver_height;
    *size = conver_size;
    // printf("conver_width: %d, conver_height: %d\n", conver_width, conver_height);

END:
    if (rendert_tex)
        GUI_DestroyTexture(rendert_tex);

    return conver_data;
}

int Emu_SaveVideoScreenshot(char *path)
{
    int ret = 0;

    char parent_path[MAX_PATH_LENGTH];
    MakeParentDir(parent_path, path, MAX_PATH_LENGTH);
    CreateFolder(parent_path);

    uint32_t base_width, base_height;
    Emu_GetVideoBaseWH(&base_width, &base_height);

    uint32_t screenshot_width, screenshot_height;
    int rotate = Emu_GetVideoDisplayRotate();

    if (rotate == TYPE_DISPLAY_ROTATE_CW_90 || rotate == TYPE_DISPLAY_ROTATE_CW_270)
    {
        screenshot_width = base_height;
        screenshot_height = base_width;
    }
    else
    {
        screenshot_width = base_width;
        screenshot_height = base_height;
    }

    uint64_t screenshot_size = 0;
    uint32_t *screenshot_buf = Emu_GetVideoScreenshotData(&screenshot_width, &screenshot_height, &screenshot_size, rotate, 0);
    if (!screenshot_buf)
        return -1;

    ret = WritePngFile(path, (unsigned char *)screenshot_buf, screenshot_width, screenshot_height, 8);
    free(screenshot_buf);
    return ret;
}

static void destroyVideoTexture()
{
    int i;
    for (i = 0; i < 2; i++)
    {
        if (video_texture_bufs[i])
        {
            GUI_DestroyTexture(video_texture_bufs[i]);
            video_texture_bufs[i] = NULL;
        }
    }
    video_texture = NULL;
}

static GUI_Texture *createVideoTexture(int width, int height)
{
    destroyVideoTexture();

    video_width = width;
    video_height = height;

    int i;
    for (i = 0; i < MAX_TEXTURE_BUFS; i++)
    {
        video_texture_bufs[i] = GUI_CreateTextureFormat(width, height, core_video_pixel_format);
    }

    video_texture_index = 0;
    video_texture = video_texture_bufs[video_texture_index];

    if (!video_texture)
    {
        APP_LOG("[VIDEO] create video texture failed\n");
        return NULL;
    }

    return video_texture;
}

static void destroyOverlayTexture()
{
    if (overlay_texture)
    {
        GUI_DestroyTexture(overlay_texture);
        overlay_texture = NULL;
    }
}

static GUI_Texture *createOverlayTexture()
{
    Setting_WaitOverlayInitEnd();
    destroyOverlayTexture();

    OverlayListEntryData *overlay_data = NULL;
    if (graphics_overlay_list && graphics_config.overlay_select > 0)
    {
        LinkedListEntry *entry = LinkedListFindByNum(graphics_overlay_list, graphics_config.overlay_select - 1);
        overlay_data = (OverlayListEntryData *)LinkedListGetEntryData(entry);
    }

    if (!overlay_data || !overlay_data->image_name) // No find
        return NULL;

    char path[MAX_PATH_LENGTH];

    // Try load image from app data overlays dir
    snprintf(path, MAX_PATH_LENGTH, "%s/%s/%s", APP_DATA_DIR, OVERLAYS_DIR_NAME, overlay_data->image_name);
    overlay_texture = GUI_LoadPNGFile(path);

    // Try load image from private assets dir
    if (!overlay_texture && private_assets_dir)
    {
        snprintf(path, MAX_PATH_LENGTH, "%s/%s/%s", private_assets_dir, OVERLAYS_DIR_NAME, overlay_data->image_name);
        overlay_texture = GUI_LoadPNGFile(path);
    }

    // Try load image from public assets dir
    if (!overlay_texture && public_assets_dir)
    {
        snprintf(path, MAX_PATH_LENGTH, "%s/%s/%s", public_assets_dir, OVERLAYS_DIR_NAME, overlay_data->image_name);
        overlay_texture = GUI_LoadPNGFile(path);
    }

    if (!overlay_texture)
    {
        APP_LOG("[VIDEO] create overlay texture failed\n");
        return NULL;
    }

    return overlay_texture;
}

static int updateVideoDisplay()
{
    OverlayListEntryData *overlay_data = NULL;
    uint32_t width = 0, height = 0;
    int rotate = 0;

    if (!video_okay)
        return -1;

    // Overlay texture
    if (video_overlay_select != graphics_config.overlay_select)
        createOverlayTexture();

    if (!video_texture)
        return -1;

    // Overlay config
    if (graphics_overlay_list && graphics_config.overlay_select > 0)
    {
        LinkedListEntry *entry = LinkedListFindByNum(graphics_overlay_list, graphics_config.overlay_select - 1);
        overlay_data = (OverlayListEntryData *)LinkedListGetEntryData(entry);
    }

    // Rotate
    rotate = Emu_GetVideoDisplayRotate();
    float radian = 270 * 0.0174532925f;
    video_rotate_rad = rotate * radian;

    // Width & height
    Emu_GetVideoDisplayWH(&width, &height);

    // Coord
    video_x = GUI_SCREEN_HALF_WIDTH;
    video_y = GUI_SCREEN_HALF_HEIGHT;
    if (overlay_data)
    {
        if (overlay_data->viewport_x)
            video_x = *(overlay_data->viewport_x) + width / 2;
        if (overlay_data->viewport_y)
            video_y = *(overlay_data->viewport_y) + height / 2;
    }

    // Scale
    if (rotate == TYPE_DISPLAY_ROTATE_CW_90 || rotate == TYPE_DISPLAY_ROTATE_CW_270)
    {
        video_x_scale = (float)height / (float)video_width;
        video_y_scale = (float)width / (float)video_height;
    }
    else
    {
        video_x_scale = (float)width / (float)video_width;
        video_y_scale = (float)height / (float)video_height;
    }

    // Shader
    if (graphics_config.graphics_shader == TYPE_GRAPHICS_SHADER_LCD3X)
        video_shader = lcd3x_shader;
    else if (graphics_config.graphics_shader == TYPE_GRAPHICS_SHADER_SHARP_BILINEAR_SIMPLE)
        video_shader = sharp_bilinear_simple_shader;
    else if (graphics_config.graphics_shader == TYPE_GRAPHICS_SHADER_SHARP_BILINEAR)
        video_shader = sharp_bilinear_shader;
    else if (graphics_config.graphics_shader == TYPE_GRAPHICS_SHADER_ADVANCED_AA)
        video_shader = advanced_aa_shader;
    else
        video_shader = NULL;

    // Filters
    int i;
    for (i = 0; i < MAX_TEXTURE_BUFS; i++)
    {
        if (video_texture_bufs[i])
        {
            if (graphics_config.graphics_smooth)
                GUI_SetTextureFilter(video_texture_bufs[i], GUI_TEXTURE_FILTER_LINEAR, GUI_TEXTURE_FILTER_LINEAR);
            else
                GUI_SetTextureFilter(video_texture_bufs[i], GUI_TEXTURE_FILTER_POINT, GUI_TEXTURE_FILTER_POINT);
        }
    }

    return 0;
}

static void refreshFps()
{
    uint64_t cur_micros = sceKernelGetProcessTimeWide();
    uint64_t interval_micros = cur_micros - last_fps_micros;
    if (interval_micros > 1000000)
    {
        last_fps_micros = cur_micros;
        video_fps = (video_frames / (double)interval_micros) * 1000000.0f;
        video_frames = 0;
    }
    video_frames++;
}

static void checkFrameDelay()
{
    uint64_t cur_micros = sceKernelGetProcessTimeWide();
    uint64_t interval_micros = cur_micros - last_frame_micros;
    if (interval_micros < micros_per_frame)
    {
        uint64_t delay_micros = micros_per_frame - interval_micros;
        sceKernelDelayThread(delay_micros);
        last_frame_micros = cur_micros + delay_micros;
    }
    else
    {
        last_frame_micros = cur_micros;
    }
}

void Emu_BeforeDrawVideo()
{
    if (!video_okay)
        return;

    GUI_LockDrawMutex();

    if (video_display_need_update)
    {
        updateVideoDisplay();
        video_display_need_update = 0;
    }

    GUI_UnlockDrawMutex();
}

void Emu_DrawVideo()
{
    if (!video_okay)
        return;

    if (overlay_texture && graphics_config.overlay_mode == TYPE_GRAPHICS_OVERLAY_MODE_BACKGROUND)
        GUI_DrawTexture(overlay_texture, 0.0f, 0.0f);

    if (video_texture)
    {
        if (video_shader)
            GUI_DrawTextureShaderPartScalRotate(video_texture, video_shader, video_x, video_y, 0, 0, video_width, video_height, video_x_scale, video_y_scale, video_rotate_rad);
        else
            GUI_DrawTexturePartScaleRotate(video_texture, video_x, video_y, 0, 0, video_width, video_height, video_x_scale, video_y_scale, video_rotate_rad);
    }

    if (overlay_texture && graphics_config.overlay_mode == TYPE_GRAPHICS_OVERLAY_MODE_OVERLAY)
        GUI_DrawTexture(overlay_texture, 0.0f, 0.0f);
}

void Emu_DrawVideoWidgets()
{
    if (graphics_config.show_fps)
    {
        refreshFps();
        GUI_DrawTextf(0.0f, 0.0f, COLOR_WHITE, "FPS: %.2f", video_fps);
    }

    if (Emu_GetRunSpeed() != 1.0f)
    {
        char fps_scale_string[12];
        snprintf(fps_scale_string, 12, "%.2fX", Emu_GetRunSpeed());
        GUI_DrawText(GUI_SCREEN_WIDTH - GUI_GetTextWidth(fps_scale_string), 0.0f, COLOR_WHITE, fps_scale_string);
    }

    if (show_player_micros > 0)
    {
        uint64_t cur_micros = sceKernelGetProcessTimeWide();
        if (cur_micros < show_player_micros)
            GUI_DrawTextf(0.0f, GUI_SCREEN_HEIGHT - GUI_GetLineHeight(), COLOR_WHITE, "%s: %dP", cur_lang[LANG_CONTROLLER_PORT], control_config.controller_port + 1);
        else
            show_player_micros = 0;
    }
}

void Retro_VideoRefreshCallback(const void *data, unsigned width, unsigned height, size_t pitch)
{
    if (!video_okay)
        return;

    if (video_pause)
        goto SIGNAL_EXIT;

    int next_texture_index = (video_texture_index + 1) % MAX_TEXTURE_BUFS;
    GUI_Texture *next_texture = video_texture_bufs[next_texture_index];

    if (video_width != width || video_height != height || !next_texture ||
        GUI_GetTextureFormat(next_texture) != core_video_pixel_format)
    {
        GUI_LockDrawMutex();
        createVideoTexture(width, height);
        updateVideoDisplay();
        GUI_UnlockDrawMutex();
        next_texture_index = video_texture_index;
        next_texture = video_texture;
    }

    if (!data || pitch <= 0)
        goto NEXT_FRAME;
    else if (next_texture && GUI_GetTextureDatap(next_texture) == data)
        goto SET_NEXT_TEXTURE;

    if (next_texture)
    {
        const uint8_t *in_data = (const uint8_t *)data;
        uint8_t *out_data = (uint8_t *)GUI_GetTextureDatap(next_texture);
        unsigned int in_pitch = pitch;
        unsigned int out_pitch = GUI_GetTextureStride(next_texture);

        int i;
        for (i = 0; i < height; i++)
        {
            memcpy(out_data, in_data, in_pitch);
            in_data += in_pitch;
            out_data += out_pitch;
        }
    }

SET_NEXT_TEXTURE:
    GUI_LockDrawMutex();
    video_texture_index = next_texture_index;
    video_texture = next_texture;
    GUI_UnlockDrawMutex();

NEXT_FRAME:
    checkFrameDelay();

SIGNAL_EXIT:
    Emu_SignalVideoSema();
}

int Emu_InitVideo()
{
    APP_LOG("[VIDEO] Video init...\n");

    video_okay = 0;
    video_pause = 1;

    if (!createVideoTexture(core_system_av_info.geometry.base_width, core_system_av_info.geometry.base_height))
        return -1;

    createOverlayTexture();
    video_semaid = sceKernelCreateSema("emu_video_sema", 0, 0, 1, NULL);

    if (control_config.controller_port != 0)
        Emu_ShowControllerPortToast();

    APP_LOG("[VIDEO] max width: %d, max height: %d\n", core_system_av_info.geometry.max_width, core_system_av_info.geometry.max_height);
    APP_LOG("[VIDEO] base width: %d, base height: %d\n", core_system_av_info.geometry.base_width, core_system_av_info.geometry.base_height);
    APP_LOG("[VIDEO] FPS: %.2f\n", core_system_av_info.timing.fps);
    APP_LOG("[VIDEO] Video init OK!\n");

    video_okay = 1;
    GUI_LockDrawMutex();
    updateVideoDisplay();
    GUI_UnlockDrawMutex();

    return 0;
}

int Emu_DeinitVideo()
{
    APP_LOG("[VIDEO] Video deinit...\n");

    video_okay = 0;
    video_pause = 1;

    destroyVideoTexture();
    destroyOverlayTexture();
    if (video_semaid >= 0)
    {
        sceKernelDeleteSema(video_semaid);
        video_semaid = -1;
    }

    GUI_SetVblankWait(1);

    APP_LOG("[VIDEO] Video deinit OK!\n");
    return 0;
}

int Emu_SignalVideoSema()
{
    return sceKernelSignalSema(video_semaid, 1);
}

int Emu_WaitVideoSema()
{
    return sceKernelWaitSema(video_semaid, 1, NULL);
}

void Emu_SetMicrosPerFrame(uint64_t micros)
{
    micros_per_frame = micros;
}

uint64_t Emu_GetMicrosPerFrame()
{
    return micros_per_frame;
}
