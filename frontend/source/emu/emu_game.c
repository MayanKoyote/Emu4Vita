#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <psp2/kernel/processmgr.h>

#include "activity/activity.h"
#include "setting/setting.h"
#include "emu/emu.h"
#include "file.h"
#include "config.h"
#include "boot.h"
#include "utils.h"
#include "lang.h"
#include "app.h"

#define MAX_GAME_RUN_SPEED 2.0f
#define STEP_GAME_RUN_SPEED 0.5f

static SceKernelLwMutexWork game_run_mutex = {0};
static EmuGameInfo game_info = {0};
static SceUID game_run_thid = -1;
static int game_run = 0, game_running = 0, game_exiting = 0, game_loading = 0, game_reloading = 0, game_loaded = 0;
static EmuGameEventAction game_event_action = EMU_GAME_EVENT_ACTION_NONE;
static float game_run_speed = 1.0f;
static void *game_rom_data = NULL;

int Emu_MakeCurrentGameName(char *name)
{
    return MakeFileName(name, game_info.path, MAX_NAME_LENGTH);
}

int Emu_MakeCurrentGamePath(char *path)
{
    strcpy(path, game_info.path);
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

void Emu_SetRunSpeed(float speed)
{
    game_run_speed = speed;
    Emu_SetMicrosPerFrame(1000000.0f / (core_system_av_info.timing.fps * speed));
}

void Emu_SetGameEventAction(EmuGameEventAction action)
{
    game_event_action = action;
}

float Emu_GetRunSpeed()
{
    return game_run_speed;
}

EmuGameEventAction Emu_GetGameEventAction()
{
    return game_event_action;
}

void Emu_WaitGameEventDone()
{
    while (game_event_action != EMU_GAME_EVENT_ACTION_NONE)
        sceKernelDelayThread(1000);
}

void Emu_GameSpeedUp()
{
    float speed = Emu_GetRunSpeed();
    if (speed < MAX_GAME_RUN_SPEED)
        speed += STEP_GAME_RUN_SPEED;
    else
        speed = 1.0f;
    Emu_SetRunSpeed(speed);
}

void Emu_GameSpeedDown()
{
    float speed = Emu_GetRunSpeed();
    if (speed > 1.0f)
        speed -= STEP_GAME_RUN_SPEED;
    else
        speed = MAX_GAME_RUN_SPEED;
    Emu_SetRunSpeed(speed);
}

void Emu_ControllerPortUp()
{
    if (control_config.controller_port < N_CTRL_PORTS - 1)
        control_config.controller_port++;
    else
        control_config.controller_port = 0;
    SaveControlConfig(TYPE_CONFIG_GAME);
    Emu_ShowControllerPortToast();
}

void Emu_ControllerPortDown()
{
    if (control_config.controller_port > 0)
        control_config.controller_port--;
    else
        control_config.controller_port = N_CTRL_PORTS - 1;
    SaveControlConfig(TYPE_CONFIG_GAME);
    Emu_ShowControllerPortToast();
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
        Emu_CleanAudio();
    }
}

void Emu_RunRetro()
{
    if (game_run)
        retro_run();
}

void Emu_RunGame()
{
    Emu_LockRunGameMutex();
    Emu_PollInput();
    Emu_RunRetro();
    Emu_UnlockRunGameMutex();
}

int Emu_EventGame()
{
    if (game_event_action == EMU_GAME_EVENT_ACTION_NONE)
        return 0;

    switch (game_event_action)
    {
    case EMU_GAME_EVENT_ACTION_OPEN_SETTING_MENU:
    {
        Emu_PauseGame();
        Setting_OpenMenu();
        UnlockQuickMenu();
    }
    break;
    case EMU_GAME_EVENT_ACTION_SAVE_STATE:
    {
        Emu_LockRunGameMutex();
        Emu_SaveState(Setting_GetStateSelectId());
        Emu_UnlockRunGameMutex();
    }
    break;
    case EMU_GAME_EVENT_ACTION_LOAD_STATE:
    {
        Emu_LockRunGameMutex();
        Emu_LoadState(Setting_GetStateSelectId());
        Emu_UnlockRunGameMutex();
    }
    break;
    case EMU_GAME_EVENT_ACTION_REWIND:
    {
        Emu_LockRunGameMutex();
        Emu_RewindGame();
        Emu_UnlockRunGameMutex();
    }
    break;
    case EMU_GAME_EVENT_ACTION_RESET:
    {
        Emu_LockRunGameMutex();
        Emu_ResetGame();
        Emu_UnlockRunGameMutex();
    }
    break;
    case EMU_GAME_EVENT_ACTION_RELOAD:
    {
        Emu_ReloadGame();
    }
    break;
    case EMU_GAME_EVENT_ACTION_EXIT:
    {
        Emu_ExitGame();
        if (BootGetMode() == BOOT_MODE_GAME)
            AppExitToArch();
    }
    break;
    case EMU_GAME_EVENT_ACTION_SPEED_UP:
    {
        Emu_GameSpeedUp();
    }
    break;
    case EMU_GAME_EVENT_ACTION_SPEED_DOWN:
    {
        Emu_GameSpeedDown();
    }
    break;
    case EMU_GAME_EVENT_ACTION_CONTROLLER_PORT_UP:
    {
        Emu_ControllerPortUp();
    }
    break;
    case EMU_GAME_EVENT_ACTION_CONTROLLER_PORT_DOWN:
    {
        Emu_ControllerPortDown();
    }
    break;
    case EMU_GAME_EVENT_ACTION_CHANGE_DISK_IMAGE_INDEX:
    {
        Emu_LockRunGameMutex();
        Emu_DiskChangeImageIndex(Setting_GetDiskImageIndex());
        Emu_UnlockRunGameMutex();
    }
    break;
    default:
        break;
    }

    game_event_action = EMU_GAME_EVENT_ACTION_NONE;
    return 0;
}

int Emu_LockRunGameMutex()
{
    return sceKernelLockLwMutex(&game_run_mutex, 1, NULL);
}

int Emu_UnlockRunGameMutex()
{
    return sceKernelUnlockLwMutex(&game_run_mutex, 1);
}

static int loadGameFromFile(const char *path)
{
    const char *rom_path = path;

#if defined(WANT_ARCHIVE_ROM)
    char cache_path[MAX_PATH_LENGTH];
    const char *ext = strrchr(path, '.');
    if (ext++ && !Emu_IsValidExtension(ext)) // 非原生格式游戏，尝试压缩包读取
    {
        ArchiveRomDriver *driver = Archive_GetDriver(ext);
        if (Archive_GetRomPath(path, cache_path, driver) < 0)
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

static int loadGameFromMemory(const char *path)
{
    size_t size = 0;

    if (game_rom_data)
    {
        free(game_rom_data);
        game_rom_data = NULL;
    }

#if defined(WANT_ARCHIVE_ROM)
    const char *ext = strrchr(path, '.');
    if (ext++ && !Emu_IsValidExtension(ext)) // 非原生格式游戏，尝试压缩包读取
    {
        ArchiveRomDriver *driver = Archive_GetDriver(ext);
        if (Archive_GetRomMemory(path, &game_rom_data, &size, driver) < 0)
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

int Emu_LoadGame()
{
    int ret = 0;

    game_loading = 1;
    APP_LOG("[GAME] Game load: %s\n", game_info.path);

    GUI_SetControlEnabled(0); // 禁止按键以避免主线程的其它操作
    Splash_SetCtrlEnabled(0); // 禁用splash_activity的按键
    Splash_SetLogEnabled(app_config.show_log);
    char splash_path[MAX_PATH_LENGTH];
    MakeSplashPath(splash_path);
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
    game_event_action = EMU_GAME_EVENT_ACTION_NONE;
    retro_init();

    if (core_system_info.need_fullpath)
        ret = loadGameFromFile(game_info.path);
    else
        ret = loadGameFromMemory(game_info.path);

    if (ret < 0)
    {
        APP_LOG("[GAME] Game load failed!\n");
        // retro_unload_game();
        retro_deinit();

        GUI_SetControlEnabled(1);
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
    Emu_LoadSrm(); // Load srm
    retro_run();   // Run one frame to fix some bug for savestate

    if (!game_reloading) // 重载游戏的话不加载存档
    {
        int state_num = game_info.state_num;
        if (state_num == -2) // -2 auto check, < -2 disable
        {
            if (misc_config.auto_save_load)
                state_num = -1;
        }
        if (state_num >= -1)
            Emu_LoadState(state_num);
    }

    Emu_InitCheat();
    Emu_InitAudio();
    Emu_InitVideo();
    Emu_InitInput();
    Emu_InitRewind();

    Retro_UpdateCoreOptionsDisplay();
    GUI_StartActivity(&emu_activity);
    GUI_SetControlEnabled(1);
    GUI_FinishActivity(&splash_activity);

    APP_LOG("[GAME] Game load OK!\n");
    game_loading = 0;
    game_loaded = 1;

    return ret;
}

int Emu_UnloadGame()
{
    if (game_loaded)
    {
        game_exiting = 1;
        APP_LOG("[GAME] Game unload...\n");

        Emu_PauseGame(); // 需要先暂停游戏以确保无线程调用Emu_LockRunGameMutex以避免造成死锁
        Emu_LockRunGameMutex();

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

        AlertDialog_Close(dialog);
        GUI_FinishActivity(&emu_activity);

        Emu_DeinitAudio();
        Emu_DeinitVideo();
        Emu_DeinitInput();
        Emu_DeinitCheat();
        Emu_DeinitRewind();

        Emu_UnlockRunGameMutex();
        sceKernelDeleteLwMutex(&game_run_mutex);

        APP_LOG("[GAME] Game unload OK!\n");
        game_exiting = 0;
        game_loaded = 0;
    }

    return 0;
}

int Emu_ReloadGame()
{
    if (game_loaded)
    {
        game_reloading = 1;
        Emu_UnloadGame();
        Emu_LoadGame();
        game_reloading = 0;
    }

    return 0;
}

/*
    游戏需要切换到线程运行，或把GUI切换到线程运行，因为有些核心在retro_run时会输出不止1帧，
    如果运行在同一线程的话会导致帧丢失。
*/
static int GameRunThreadEntry(SceSize args, void *argp)
{
    if (Emu_LoadGame() < 0)
        return -1;

    game_run = 1;
    Emu_ResumeGame();

    while (game_run)
    {
        if (game_running)
            Emu_RunGame();

        Emu_EventGame();
    }

    Emu_UnloadGame();

    sceKernelExitThread(0);
    return 0;
}

int Emu_StartGame(EmuGameInfo *info)
{
    if (!info)
        return -1;

    Emu_ExitGame();
    Emu_WaitGameExitEnd();

    memcpy(&game_info, info, sizeof(EmuGameInfo));

    int ret = 0;

    // 游戏线程创建后不能删除，一些核心(如PCSX-ReARMed)如果运行过一次游戏后删除了该线程再重新创建，游戏将无法运行会崩溃，原因暂不明
    if (game_run_thid < 0)
        ret = game_run_thid = sceKernelCreateThread("emu_game_run_thread", GameRunThreadEntry, 0x10000100, 0x40000, 0, 0, NULL);
    if (game_run_thid >= 0)
        ret = sceKernelStartThread(game_run_thid, 0, NULL);

    return ret;
}

int Emu_ExitGame()
{
    game_run = 0;
    return 0;
}

int Emu_WaitGameExitEnd()
{
    if (game_run_thid >= 0)
        sceKernelWaitThreadEnd(game_run_thid, NULL, NULL);

    return 0;
}

int Emu_FinishGame()
{
    game_run = 0;
    if (game_run_thid >= 0)
    {
        sceKernelWaitThreadEnd(game_run_thid, NULL, NULL);
        sceKernelDeleteThread(game_run_thid);
        game_run_thid = -1;
    }
    return 0;
}
