#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <psp2/kernel/processmgr.h>
#include <psp2/io/fcntl.h>

#include "activity/activity.h"
#include "setting/setting.h"
#include "emu/emu.h"
#include "file.h"
#include "config.h"
#include "boot.h"
#include "utils.h"
#include "lang.h"

#define MAX_GAME_RUN_SPEED 2.0f
#define STEP_GAME_RUN_SPEED 0.5f

static EmuGameInfo game_info = {0};
static int game_running = 0, game_exiting = 0, game_loading = 0, game_reloading = 0, game_loaded = 0;
static int game_run_event_action_type = TYPE_GAME_RUN_EVENT_ACTION_NONE;
static float game_run_speed = 1.0f;
static double game_cur_fps = 0;
static void *game_rom_data = NULL;
static SceKernelLwMutexWork game_run_mutex = {0};

static int makeSplashPicPath(char *path)
{
    char name[MAX_NAME_LENGTH];
    MakeCurrentFileName(name);
    char base_name[MAX_NAME_LENGTH];
    MakeBaseName(base_name, name, MAX_NAME_LENGTH);
    snprintf(path, MAX_PATH_LENGTH, "%s/%s.png", (CORE_SPLASHS_DIR), base_name);
    return 0;
}

int Emu_IsGameRunning()
{
    return game_running;
}

int Emu_IsGameLoading()
{
    return game_loading;
}

int Emu_IsGameLoaded()
{
    return game_loaded;
}

int Emu_IsGameExiting()
{
    return game_exiting;
}

double Emu_GetCurrentFps()
{
    return game_cur_fps;
}

float Emu_GetCurrentRunSpeed()
{
    return game_run_speed;
}

void Emu_SetRunSpeed(float speed)
{
    game_run_speed = speed;
    game_cur_fps = core_system_av_info.timing.fps * (double)speed;
    Emu_SetMicrosPerFrame(1000000.0f / game_cur_fps);
}

void Emu_SpeedUpGame()
{
    float speed = Emu_GetCurrentRunSpeed();
    if (speed < MAX_GAME_RUN_SPEED)
        speed += STEP_GAME_RUN_SPEED;
    else
        speed = 1.0f;
    Emu_SetRunSpeed(speed);
}

void Emu_SpeedDownGame()
{
    float speed = Emu_GetCurrentRunSpeed();
    if (speed > 1.0f)
        speed -= STEP_GAME_RUN_SPEED;
    else
        speed = MAX_GAME_RUN_SPEED;
    Emu_SetRunSpeed(speed);
}

static int loadGameFromFile(const char *path, int type)
{
    const char *rom_path = path;

#if defined(WANT_EXT_ARCHIVE_ROM)
    char cache_path[MAX_PATH_LENGTH];
    if (type >= n_core_valid_extensions)
    {
        if (Archive_GetRomPath(path, cache_path, Archive_GetDriver(type - n_core_valid_extensions)) < 0)
            return -1;
        rom_path = cache_path;
    }
#endif

    struct retro_game_info game_info;
    game_info.path = rom_path;
    game_info.data = NULL;
    game_info.size = 0;
    game_info.meta = NULL;
    int ret = retro_load_game(&game_info);
    if (!ret)
        return -1;

    return 0;
}

static int loadGameFromMemory(const char *path, int type)
{
    if (game_rom_data)
    {
        free(game_rom_data);
        game_rom_data = NULL;
    }

    size_t size = 0;

#if defined(WANT_EXT_ARCHIVE_ROM)
    if (type >= n_core_valid_extensions)
    {
        if (Archive_GetRomMemory(path, &game_rom_data, &size, Archive_GetDriver(type - n_core_valid_extensions)) < 0)
            return -1;
    }
    else
#endif
    {
        if (AllocateReadFileEx(path, &game_rom_data, &size) < 0)
            return -1;
    }

    struct retro_game_info game_info;
    game_info.path = path;
    game_info.data = game_rom_data;
    game_info.size = size;
    game_info.meta = NULL;

    int ret = retro_load_game(&game_info);
    if (!ret)
    {
        free(game_rom_data);
        game_rom_data = NULL;
        return -1;
    }

    return 0;
}

int Emu_StartGame(EmuGameInfo *info)
{
    if (!info)
        return -1;

    Emu_ExitGame();
    memcpy(&game_info, info, sizeof(EmuGameInfo));

    int ret = 0;

    AppLog("[GAME] Game load: %s\n", game_info.path);

    game_loading = 1;

    GUI_StartAsyncDraw();
    Splash_SetCtrlEnabled(0);
    Splash_SetLogEnabled(app_config.show_log);
    char splash_path[MAX_PATH_LENGTH];
    makeSplashPicPath(splash_path);
    GUI_Texture *texture = GUI_LoadPNGFile(splash_path);
    Splash_SetBgTexture(texture);
    GUI_StartActivity(&splash_activity);

    if (!game_reloading)
    {
        LoadGraphicsConfig(TYPE_CONFIG_GAME);
        LoadControlConfig(TYPE_CONFIG_GAME);
        LoadHotkeyConfig(TYPE_CONFIG_GAME);
        LoadMiscConfig(TYPE_CONFIG_GAME);
        LoadCoreConfig(TYPE_CONFIG_GAME);
    }

    core_display_rotate = 0;
    game_run_event_action_type = TYPE_GAME_RUN_EVENT_ACTION_NONE;
    retro_init();

    if (core_system_info.need_fullpath)
        ret = loadGameFromFile(game_info.path, game_info.type);
    else
        ret = loadGameFromMemory(game_info.path, game_info.type);

    if (ret < 0)
    {
        AppLog("[GAME] Game load failed!\n");
        // retro_unload_game();
        retro_deinit();

        GUI_FinishAsyncDraw();
        Splash_SetCtrlEnabled(1);
        if (!app_config.show_log)
            GUI_FinishActivity(&splash_activity);
        AlertDialog_ShowSimpleDialog(cur_lang[LANG_TIP], cur_lang[LANG_MESSAGE_START_GAME_FAILED]);
        return -1;
    }

    WriteFile((LASTFILE_PATH), game_info.path, strlen(game_info.path) + 1);

    sceKernelCreateLwMutex(&game_run_mutex, "game_run_mutex", 2, 0, NULL);
    retro_get_system_av_info(&core_system_av_info);
    Emu_SetRunSpeed(1.0f);
    Retro_SetControllerPortDevices();
    Emu_LoadSrm(); // Auto load srm
    retro_run();   // Run one frame to fix some bug for savestate

    int state_num = game_info.state_num;
    if (state_num == -2) // -2 auto check, < -2 disable
    {
        if (misc_config.auto_save_load)
            state_num = -1;
    }
    if (state_num >= -1)
        Emu_LoadState(state_num);

    Emu_InitCheat();
    Emu_InitAudio();
    Emu_InitVideo();
    Emu_InitInput();
    Emu_InitRewind();

    Retro_UpdateCoreOptionsDisplay();
    GUI_StartActivity(&emu_activity);
    GUI_FinishActivity(&splash_activity);

    game_loaded = 1;
    game_loading = 0;
    Emu_ResumeGame();

    AppLog("[GAME] Game load OK!\n");

    return ret;
}

void Emu_ExitGame()
{
    if (game_loaded)
    {
        AppLog("[GAME] Game unload...\n");

        Emu_LockRunGameMutex();
        game_exiting = 1;
        Emu_PauseGame();

        AlertDialog *dialog = AlertDialog_Create();
        AlertDialog_SetMessage(dialog, cur_lang[LANG_MESSAGE_WAIT_EXITING]);
        AlertDialog_Open(dialog);

        Emu_SaveSrm(); // Auto save srm
        if (!game_reloading && misc_config.auto_save_load)
        { // Auto save state
            Emu_SaveState(-1);
            Browser_RequestRefreshPreview(1);
        }
        retro_unload_game();
        retro_deinit();

        if (game_rom_data)
        {
            free(game_rom_data);
            game_rom_data = NULL;
        }

        if (!game_reloading)
        {
            LoadGraphicsConfig(TYPE_CONFIG_MAIN);
            LoadControlConfig(TYPE_CONFIG_MAIN);
            LoadHotkeyConfig(TYPE_CONFIG_MAIN);
            LoadMiscConfig(TYPE_CONFIG_MAIN);
            LoadCoreConfig(TYPE_CONFIG_MAIN);
            Retro_UpdateCoreOptionsDisplay();
        }

        GUI_FinishAsyncDraw();
        AlertDialog_Close(dialog);
        GUI_FinishActivity(&emu_activity);

        Emu_DeinitAudio();
        Emu_DeinitVideo();
        Emu_DeinitInput();
        Emu_DeinitCheat();
        Emu_DeinitRewind();

        game_loaded = 0;
        game_exiting = 0;
        Emu_UnlockRunGameMutex();
        sceKernelDeleteLwMutex(&game_run_mutex);

        AppLog("[GAME] Game unload OK!\n");
    }
}

void Emu_PauseGame()
{
    if (game_loaded)
    {
        game_running = 0;
        Emu_PauseCheat();
        Emu_PauseRewind();
        Emu_PauseAudio();
        Emu_PauseVideo();
    }
}

void Emu_ResumeGame()
{
    if (game_loaded)
    {
        game_running = 1;
        Emu_ResumeCheat();
        Emu_ResumeRewind();
        Emu_ResumeAudio();
        Emu_ResumeVideo();
        LockQuickMenu();
    }
}

void Emu_ResetGame()
{
    if (game_loaded)
    {
        retro_reset();
        Emu_CleanAudioSound();
    }
}

int Emu_ReloadGame()
{
    if (game_loaded)
    {
        game_reloading = 1;
        Emu_ExitGame();

        EmuGameInfo info;
        MakeCurrentFilePath(info.path);
        info.type = GetCurrentFileType();
        info.state_num = -3;
        Emu_StartGame(&info);

        game_reloading = 0;
    }

    return 0;
}

void Emu_SetGameRunEventAction(int type)
{
    game_run_event_action_type = type;
}

void Emu_RunGame()
{
    Emu_LockRunGameMutex();
    Emu_PollInput();
    retro_run();
    Emu_UnlockRunGameMutex();
    Emu_EventRunGame();
}

int Emu_EventRunGame()
{
    if (game_run_event_action_type == TYPE_GAME_RUN_EVENT_ACTION_NONE)
        return 0;

    Emu_LockRunGameMutex();

    switch (game_run_event_action_type)
    {
    case TYPE_GAME_RUN_EVENT_ACTION_SAVE_STATE:
    {
        Emu_SaveState(Setting_GetStateSelectId());
    }
    break;
    case TYPE_GAME_RUN_EVENT_ACTION_LOAD_STATE:
    {
        Emu_LoadState(Setting_GetStateSelectId());
    }
    break;
    case TYPE_GAME_RUN_EVENT_ACTION_REWIND:
    {
        Emu_RewindGame();
    }
    break;
    case TYPE_GAME_RUN_EVENT_ACTION_RESET:
    {
        Emu_ResetGame();
    }
    break;
    case TYPE_GAME_RUN_EVENT_ACTION_EXIT:
    {
        Emu_UnlockRunGameMutex();
        Emu_ExitGame();
        if (BootGetMode() == BOOT_MODE_GAME)
            BootLoadParentExec();
        return 0;
    }
    break;
    case TYPE_GAME_RUN_EVENT_ACTION_OPEN_SETTING_MENU:
    {
        Emu_PauseGame();
        Setting_OpenMenu();
        UnlockQuickMenu();
    }
    break;
    default:
        break;
    }

    game_run_event_action_type = TYPE_GAME_RUN_EVENT_ACTION_NONE;
    Emu_UnlockRunGameMutex();
    return 0;
}

void Emu_LockRunGameMutex()
{
    sceKernelLockLwMutex(&game_run_mutex, 1, NULL);
}

void Emu_UnlockRunGameMutex()
{
    sceKernelUnlockLwMutex(&game_run_mutex, 1);
}
