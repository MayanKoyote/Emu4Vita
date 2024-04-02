#ifndef __M_EMU_GAME_H__
#define __M_EMU_GAME_H__

#include "file.h"

typedef enum EmuGameEventAction
{
    EMU_GAME_EVENT_ACTION_NONE,
    EMU_GAME_EVENT_ACTION_OPEN_SETTING_MENU,
    EMU_GAME_EVENT_ACTION_SAVE_STATE,
    EMU_GAME_EVENT_ACTION_LOAD_STATE,
    EMU_GAME_EVENT_ACTION_REWIND,
    EMU_GAME_EVENT_ACTION_RESET,
    EMU_GAME_EVENT_ACTION_RELOAD,
    EMU_GAME_EVENT_ACTION_EXIT,
    EMU_GAME_EVENT_ACTION_SPEED_UP,
    EMU_GAME_EVENT_ACTION_SPEED_DOWN,
    EMU_GAME_EVENT_ACTION_CONTROLLER_PORT_UP,
    EMU_GAME_EVENT_ACTION_CONTROLLER_PORT_DOWN,
    EMU_GAME_EVENT_ACTION_CHANGE_DISK_IMAGE_INDEX,
} EmuGameEventAction;

typedef struct EmuGameInfo
{
    char path[MAX_PATH_LENGTH];
    int state_num; // -1自动存档，-2自动判断，<-2禁用存档
} EmuGameInfo;

int Emu_MakeCurrentGameName(char *name);
int Emu_MakeCurrentGamePath(char *path);

int Emu_IsGameRunning();
int Emu_IsGameLoading();
int Emu_IsGameLoaded();
int Emu_IsGameExiting();

void Emu_SetRunSpeed(float speed);
void Emu_SetGameEventAction(EmuGameEventAction action);

float Emu_GetRunSpeed();
EmuGameEventAction Emu_GetGameEventAction();

void Emu_WaitGameEventDone();

int Emu_EventGame();
void Emu_RunGame();
void Emu_PauseGame();
void Emu_ResumeGame();
void Emu_ResetGame();
int Emu_LoadGame();
int Emu_UnloadGame();
int Emu_ReloadGame();
void Emu_GameSpeedUp();
void Emu_GameSpeedDown();
void Emu_ControllerPortUp();
void Emu_ControllerPortDown();

int Emu_LockRunGameMutex();
int Emu_UnlockRunGameMutex();

int Emu_StartGame(EmuGameInfo *info);
int Emu_ExitGame();
int Emu_WaitGameExitEnd();
int Emu_FinishGame();

#endif