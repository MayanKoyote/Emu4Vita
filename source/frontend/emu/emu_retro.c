#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <psp2/display.h>
#include <psp2/io/fcntl.h>
#include <pthread.h>
#include <time/rtime.h>

#include "emu/emu.h"
#include "utils.h"
#include "file.h"
#include "config.h"

extern bool Retro_EnvironmentCallback(unsigned int cmd, void *data);
extern size_t Retro_AudioSampleBatchCallback(const int16_t *data, size_t frames);
extern void Retro_VideoRefreshCallback(const void *data, unsigned width, unsigned height, size_t pitch);
extern void Retro_InputPollCallback();
extern int16_t Retro_InputStateCallback(unsigned port, unsigned device, unsigned index, unsigned id);

char core_assets_dir[MAX_PATH_LENGTH];
char core_system_dir[MAX_PATH_LENGTH];

struct retro_core_options_update_display_callback *core_options_update_display_callback = NULL;
struct retro_disk_control_ext_callback *core_disk_control_ext_callback = NULL;

struct retro_system_info core_system_info = {0};
struct retro_system_av_info core_system_av_info = {0};
enum retro_pixel_format core_pixel_format = RETRO_PIXEL_FORMAT_RGB565;
GuiPixelFormat core_video_pixel_format = GUI_PIXEL_FORMAT_U5U6U5_RGB;
int core_input_supports_bitmasks = 0;
int core_display_rotate = 0;
int core_want_ext_zip_mode = 0;
int core_want_ext_7z_mode = 0;

static unsigned int emu_device_type = RETRO_DEVICE_JOYPAD;

static int makeCoreAssetsDirPath(char *path)
{
    snprintf(path, MAX_PATH_LENGTH, "%s/%s", APP_DATA_DIR, CORE_ASSETS_DIR_NAME);
    return 0;
}

static int makeCoreSystemDirPath(char *path)
{
    snprintf(path, MAX_PATH_LENGTH, "%s/%s", APP_DATA_DIR, CORE_SYSTEM_DIR_NAME);
    return 0;
}

static void freeValidFileExts()
{
    if (file_valid_exts)
    {
        int i;
        for (i = 0; file_valid_exts[i]; i++)
            free(file_valid_exts[i]);
        free(file_valid_exts);
        file_valid_exts = NULL;
    }
}

static int creatValidFileExts()
{
    int ret = 0;
    const char *exts = core_system_info.valid_extensions;
    int exts_len;
    int n_exts;
    int i;
    const char *zip_ext = "zip";
    const char *sevenz_ext = "7z";

#if defined(FC_BUILD) || defined(SFC_BUILD) || defined(GBC_BUILD) || defined(GBA_BUILD) || \
    defined(MD_BUILD) || defined(NGP_BUILD) || defined(WSC_BUILD) || defined(PCE_BUILD)
    core_want_ext_zip_mode = 1;
    core_want_ext_7z_mode = 1;
#else
    core_want_ext_zip_mode = 0;
    core_want_ext_7z_mode = 0;
#endif

    if (!exts)
        return -1;

    AppLog("[RETRO] Valid ext: %s\n", exts);

    exts_len = strlen(exts);
    if (exts_len == 0)
        return -1;

    n_exts = 1;
    for (i = 0; i < exts_len; i++)
    {
        if (exts[i] == '|')
            n_exts++;
    }
    // printf("n_exts: %d\n", n_exts);

    if (file_valid_exts)
        freeValidFileExts();
    // +3 for "zip" + "7z" + NULL
    file_valid_exts = (char **)calloc((n_exts + 3), sizeof(char *));
    if (!file_valid_exts)
        return -1;

    const char *p = exts;
    const char *sep;
    int len;
    for (i = 0; i < n_exts; i++)
    {
        sep = strchr(p, '|');
        if (!sep)
            sep = exts + exts_len;
        len = sep - p;
        file_valid_exts[i] = (char *)malloc(len + 1);
        if (file_valid_exts[i])
        {
            strncpy(file_valid_exts[i], p, len);
            file_valid_exts[i][len] = '\0';

            // If the core native support zip rom, skip ext zip mode
            if (core_want_ext_zip_mode && strcasecmp(file_valid_exts[i], zip_ext) == 0)
                core_want_ext_zip_mode = 0;
            // If the core native support 7z rom, skip ext 7z mode
            if (core_want_ext_7z_mode && strcasecmp(file_valid_exts[i], zip_ext) == 0)
                core_want_ext_7z_mode = 0;
        }
        p = sep + 1;
    }

    if (core_want_ext_zip_mode)
    {
        file_valid_exts[i] = (char *)malloc(strlen(zip_ext) + 1);
        strcpy(file_valid_exts[i], zip_ext);
        i++;
    }

    if (core_want_ext_7z_mode)
    {
        file_valid_exts[i] = (char *)malloc(strlen(sevenz_ext) + 1);
        strcpy(file_valid_exts[i], sevenz_ext);
        i++;
    }

    file_valid_exts[i] = NULL;

    // for (i = 0; i < n_exts; i++)
    //     printf("exts[%d]: %s\n", i, file_valid_exts[i]);

    return ret;
}

static void setRetroCallbacks()
{
    retro_set_environment(Retro_EnvironmentCallback);
    retro_set_video_refresh(Retro_VideoRefreshCallback);
    retro_set_audio_sample_batch(Retro_AudioSampleBatchCallback);
    retro_set_input_poll(Retro_InputPollCallback);
    retro_set_input_state(Retro_InputStateCallback);
}

void Retro_SetControllerPortDevices()
{
    int i;
    for (i = 0; i < 4; i++)
        retro_set_controller_port_device(i, emu_device_type);
}

int Retro_InitLib()
{
    AppLog("[RETRO] Init retro lib...\n");

    pthread_init();
    rtime_init();

    makeCoreAssetsDirPath(core_assets_dir);
    makeCoreSystemDirPath(core_system_dir);

    setRetroCallbacks();
    retro_get_system_info(&core_system_info);
    creatValidFileExts();

    AppLog("[RETRO] Init retro lib OK!\n");

    return 0;
}

int Retro_DeinitLib()
{
    AppLog("[RETRO] Deinit retro lib...\n");

    rtime_deinit();
    pthread_terminate();
    freeValidFileExts();

    AppLog("[RETRO] Deinit retro lib OK!\n");

    return 0;
}
