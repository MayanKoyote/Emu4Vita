#ifndef __M_EMU_GAME_H__
#define __M_EMU_GAME_H__

#include "file.h"

enum GameRunActionType
{
    TYPE_GAME_RUN_EVENT_ACTION_NONE,
    TYPE_GAME_RUN_EVENT_ACTION_SAVE_STATE,
    TYPE_GAME_RUN_EVENT_ACTION_LOAD_STATE,
    TYPE_GAME_RUN_EVENT_ACTION_RESET,
    TYPE_GAME_RUN_EVENT_ACTION_REWIND,
    TYPE_GAME_RUN_EVENT_ACTION_EXIT,
    TYPE_GAME_RUN_EVENT_ACTION_OPEN_SETTING_MENU,
};

typedef struct EmuGameInfo
{
    char path[MAX_PATH_LENGTH];
    int type;
    int state_num; // -1自动存档，-2自动判断，<-2禁用存档
} EmuGameInfo;

int Emu_IsGameRunning();
int Emu_IsGameLoading();
int Emu_IsGameLoaded();
int Emu_IsGameExiting();
double Emu_GetCurrentFps();
float Emu_GetCurrentRunSpeed();
void Emu_SetRunSpeed(float speed);

int Emu_StartGame(EmuGameInfo *info);
void Emu_ExitGame();
int Emu_EventRunGame();
void Emu_RunGame();
void Emu_PauseGame();
void Emu_ResumeGame();
void Emu_ResetGame();
int Emu_ReloadGame();
void Emu_SpeedUpGame();
void Emu_SpeedDownGame();

void Emu_SetGameRunEventAction(int type);
void Emu_LockRunGameMutex();
void Emu_UnlockRunGameMutex();

#endif