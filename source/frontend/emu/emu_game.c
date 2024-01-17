#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <psp2/kernel/processmgr.h>
#include <psp2/io/fcntl.h>

#include "activity/browser.h"
#include "activity/loading.h"
#include "setting/setting.h"
#include "emu/emu.h"
#include "file.h"
#include "config.h"
#include "boot.h"
#include "utils.h"
#include "lang.h"

#define MAX_GAME_RUN_SPEED 2.0f
#define STEP_GAME_RUN_SPEED 0.5f

extern GUI_Activity loading_activity;
extern GUI_Dialog setting_dialog;

static int game_loading = 0, game_loaded = 0, game_reloading = 0, game_run = 0;
static int game_run_event_action_type = TYPE_GAME_RUN_EVENT_ACTION_NONE;
static float game_run_speed = 1.0f;
static double game_cur_fps = 0;
static void *game_rom_data = NULL;

int Emu_IsGameRun()
{
    return game_run;
}

int Emu_IsGameLoading()
{
    return game_loading;
}

int Emu_IsGameLoaded()
{
    return game_loaded;
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

static int loadGameFromFile(const char *path, int archive_mode)
{
    const char *rom_path = path;
    char cache_path[MAX_PATH_LENGTH];
    if (archive_mode != ARCHIVE_MODE_NO)
    {
        if (Archive_GetRomPath(path, cache_path, archive_mode) < 0)
            return -1;
        rom_path = cache_path;
    }

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

static int loadGameFromMemory(const char *path, int archive_mode)
{
    if (game_rom_data)
        free(game_rom_data);
    game_rom_data = NULL;

    size_t size = 0;

    if (archive_mode != ARCHIVE_MODE_NO)
    {
        if (Archive_GetRomMemory(path, &game_rom_data, &size, archive_mode) < 0)
            return -1;
    }
    else
    {
        if (AllocateReadFileEX(path, &game_rom_data, &size) < 0)
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

static int loadGame(const char *path)
{
    int ret;

    AppLog("[GAME] Load game...\n");
    AppLog("[GAME] Rom path: %s\n", path);

    if (!game_reloading)
    {
        LoadGraphicsConfig(TYPE_CONFIG_GAME);
        LoadControlConfig(TYPE_CONFIG_GAME);
        LoadHotkeyConfig(TYPE_CONFIG_GAME);
        LoadMiscConfig(TYPE_CONFIG_GAME);
        LoadCoreConfig(TYPE_CONFIG_GAME);
    }

    core_display_rotate = 0;
    retro_init();

    int archive_mode = ARCHIVE_MODE_NO;

    if (core_want_ext_zip_mode || core_want_ext_7z_mode)
    {
        const char *ext = strrchr(path, '.');
        if (ext++)
        {
            if (core_want_ext_zip_mode && strcasecmp(ext, "zip") == 0)
                archive_mode = ARCHIVE_MODE_ZIP;

            if (archive_mode == ARCHIVE_MODE_NO && core_want_ext_7z_mode && strcasecmp(ext, "7z") == 0)
                archive_mode = ARCHIVE_MODE_7Z;
        }
    }

    if (core_system_info.need_fullpath)
        ret = loadGameFromFile(path, archive_mode);
    else
        ret = loadGameFromMemory(path, archive_mode);

    if (ret < 0)
    {
        AppLog("[GAME] Load game failed!\n");
        Emu_ExitGame();
    }
    else
    {
        AppLog("[GAME] Load game OK!\n");
    }
    return ret;
}

int Emu_StartGame(EmuGameInfo *info)
{
    if (!info)
        return -1;

    game_loading = 1;

    Loading_SetAutoScrollListview(1);
    Loading_StartActivityThread();

    if (loadGame(info->path) < 0)
    {
        game_loading = 0;
        Loading_SetAutoScrollListview(0);
        AlertDialog_ShowSimpleTipDialog(cur_lang[TITLE_TIP], cur_lang[MESSAGE_START_GAME_FAILED]);
        if (app_config.show_log)
            Loading_WaitActivityThreadEnd();
        else
            Loading_ExitActivityThread();
        return -1;
    }

    game_loading = 0;
    game_reloading = 0;
    WriteFile((LASTFILE_PATH), info->path, strlen(info->path) + 1);

    AppLog("[GAME] Start game...\n");

    game_run_event_action_type = TYPE_GAME_RUN_EVENT_ACTION_NONE;
    retro_get_system_av_info(&core_system_av_info);
    Emu_SetRunSpeed(1.0f);
    Retro_SetControllerPortDevices();
    Emu_LoadSrm(); // Auto load savefile
    retro_run();   // Run one frame to fix some bug for savestate
    int state_num = info->state_num;
    if (state_num == -2) // -2 auto check, < -2 disable
    {
        if (misc_config.auto_save_load)
            state_num = -1;
    }

    if (state_num >= -1)
        Emu_LoadState(state_num);

    Emu_InitCheat();

    GUI_CleanPad();
    Emu_RequestUpdateVideoDisplay();
    Retro_UpdateCoreOptionsDisplay();
    Setting_RequestUpdateMenu();

    Emu_InitAudio();
    Emu_InitVideo();
    Emu_InitInput();

    game_loaded = 1;
    Emu_ResumeGame();
    Loading_ExitActivityThread();

    AppLog("[GAME] Start game OK!\n");

    return 0;
}

void Emu_ExitGame()
{
    AppLog("[GAME] Exit game...\n");

    if (game_loaded)
    {
        Emu_PauseGame();
        Emu_SaveSrm(); // Auto save file
        if (!game_reloading && misc_config.auto_save_load)
        { // Auto save state
            Emu_SaveState(-1);
            Browser_RequestRefreshPreview(1);
        }
        retro_unload_game();
        retro_deinit();
        game_loaded = 0;
        Emu_DeinitCheat();
        Emu_DeinitAudio();
        Emu_DeinitVideo();
        Emu_DeinitInput();

        if (!game_reloading)
        {
            LoadGraphicsConfig(TYPE_CONFIG_MAIN);
            LoadControlConfig(TYPE_CONFIG_MAIN);
            LoadHotkeyConfig(TYPE_CONFIG_MAIN);
            LoadMiscConfig(TYPE_CONFIG_MAIN);
            LoadCoreConfig(TYPE_CONFIG_MAIN);
            Retro_UpdateCoreOptionsDisplay();
            Setting_RequestUpdateMenu();
        }
    }

    if (game_rom_data)
    {
        free(game_rom_data);
        game_rom_data = NULL;
    }

    if (core_cheat_list)
    {
        LinkedListDestroy(core_cheat_list);
        core_cheat_list = NULL;
    }
    Setting_SetCheatMenu(NULL);

    AppLog("[GAME] Exit game OK!\n");
}

void Emu_PauseGame()
{
    if (game_loaded)
    {
        game_run = 0;
        Emu_PauseCheat();
        Emu_PauseAudio();
        Emu_PauseVideo();
        if (game_run_event_action_type == TYPE_GAME_RUN_EVENT_ACTION_NONE)
            UnlockQuickMenu();
    }
}

void Emu_ResumeGame()
{
    if (game_loaded)
    {
        game_run = 1;
        Emu_ResumeCheat();
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
    game_reloading = 1;
    if (game_loaded)
        Emu_ExitGame();

    EmuGameInfo info;
    MakeCurrentFilePath(info.path);
    info.state_num = -3;
    Emu_StartGame(&info);

    return 0;
}

void Emu_SetGameRunEventAction(int type)
{
    game_run_event_action_type = type;
}

static void onGameRunEvent()
{
    if (game_run_event_action_type == TYPE_GAME_RUN_EVENT_ACTION_NONE)
        return;

    switch (game_run_event_action_type)
    {
    case TYPE_GAME_RUN_EVENT_ACTION_SAVE_STATE:
    {
        Emu_PauseGame();
        Emu_SaveState(Setting_GetStateSelectId());
        Emu_ResumeGame();
    }
    break;
    case TYPE_GAME_RUN_EVENT_ACTION_LOAD_STATE:
    {
        Emu_PauseGame();
        Emu_LoadState(Setting_GetStateSelectId());
        Emu_ResumeGame();
    }
    break;
    case TYPE_GAME_RUN_EVENT_ACTION_RESET:
    {
        Emu_PauseGame();
        Emu_ResetGame();
        Emu_ResumeGame();
    }
    break;
    case TYPE_GAME_RUN_EVENT_ACTION_EXIT:
    {
        Emu_ExitGame();
        if (exec_boot_mode == BOOT_MODE_GAME)
            BootLoadParentExec();
    }
    break;
    default:
        break;
    }

    game_run_event_action_type = TYPE_GAME_RUN_EVENT_ACTION_NONE;
}

void Emu_RunGame()
{
    Emu_PollInput();
    retro_run();
    onGameRunEvent();
}
