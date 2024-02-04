#ifndef __M_BOOT_H__
#define __M_BOOT_H__

enum BootModes
{
    BOOT_MODE_NONE,
    BOOT_MODE_GAME,
    BOOT_MODE_ARCH,
};

int BootLoadExec(char *app_path, char *argv[]);
int BootLoadExecForGame(char *app_path, char *game_path, char *assets_dir);
int BootLoadExecForCore(char *app_path, char *assets_dir);

#endif