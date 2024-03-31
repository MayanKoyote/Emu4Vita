#ifndef __M_BOOT_H__
#define __M_BOOT_H__

enum BootMode
{
    BOOT_MODE_NONE,
    BOOT_MODE_GAME,
    BOOT_MODE_ARCH,
};

int BootLoadExec(const char *app_path, char *const *argv);
int BootLoadExecForGame(const char *app_path, const char *game_path, const char *assets_dir);
int BootLoadExecForCore(const char *app_path, const char *assets_dir);

#endif