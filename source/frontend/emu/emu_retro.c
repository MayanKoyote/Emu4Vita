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
#if defined(FC_BUILD) || defined(SFC_BUILD) || defined(GBC_BUILD) || defined(GBA_BUILD) || \
    defined(MD_BUILD) || defined(NGP_BUILD) || defined(WSC_BUILD) || defined(PCE_BUILD)
int core_want_ext_archive_rom = 1;
#else
int core_want_ext_archive_rom = 0;
#endif

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

static void freeCoreValidExtensions()
{
    if (core_valid_extensions)
    {
        int i;
        for (i = 0; i < n_core_valid_extensions; i++)
        {
            free(core_valid_extensions[i]);
        }
        free(core_valid_extensions);
        core_valid_extensions = NULL;
    }
}

static int creatCoreValidExtensions()
{
    int ret = 0;
    const char *exts = core_system_info.valid_extensions;
    int exts_len;
    int i;

    if (!exts)
        return -1;

    AppLog("[RETRO] Valid ext: %s\n", exts);

    exts_len = strlen(exts);
    if (exts_len == 0)
        return -1;

    n_core_valid_extensions = 1;
    for (i = 0; i < exts_len; i++)
    {
        if (exts[i] == '|')
            n_core_valid_extensions++;
    }

    if (core_valid_extensions)
        freeCoreValidExtensions();
    core_valid_extensions = (char **)calloc(n_core_valid_extensions, sizeof(char *));
    if (!core_valid_extensions)
        return -1;

    const char *p = exts;
    const char *sep;
    int len;
    for (i = 0; i < n_core_valid_extensions; i++)
    {
        sep = strchr(p, '|');
        if (!sep)
            sep = exts + exts_len;
        len = sep - p;
        core_valid_extensions[i] = (char *)malloc(len + 1);
        if (core_valid_extensions[i])
        {
            strncpy(core_valid_extensions[i], p, len);
            core_valid_extensions[i][len] = '\0';
        }
        p = sep + 1;
    }

    // printf("n_core_valid_extensions: %d\n", n_core_valid_extensions);
    // for (i = 0; i < n_core_valid_extensions; i++)
    //     printf("core_valid_extensions[%d]: %s\n", i, core_valid_extensions[i]);

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
    creatCoreValidExtensions();

    AppLog("[RETRO] Init retro lib OK!\n");

    return 0;
}

int Retro_DeinitLib()
{
    AppLog("[RETRO] Deinit retro lib...\n");

    rtime_deinit();
    pthread_terminate();
    freeCoreValidExtensions();

    AppLog("[RETRO] Deinit retro lib OK!\n");

    return 0;
}
