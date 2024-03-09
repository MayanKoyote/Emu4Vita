#ifndef __M_BOOT_H__
#define __M_BOOT_H__

enum BootMode
{
    BOOT_MODE_NONE,
	BOOT_MODE_GAME,
    BOOT_MODE_ARCH,
};

int BootGetMode();
int BootLoadGame();
int BootCheckParams(int argc, char *const *argv);
int BootLoadExec(char *app_path, char *const *argv);
int BootLoadParentExec();
int BootLoadExecForGame(char *app_path, char *game_path, char *assets_dir);
int BootLoadExecForCore(char *app_path, char *assets_dir);

#endif