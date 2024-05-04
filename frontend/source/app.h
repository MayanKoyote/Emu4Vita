#ifndef __M_APP_H__
#define __M_APP_H__

extern int system_language, system_enter_button, system_date_format, system_time_format;

int IsSafeMode();
int IsVitatvModel();

int AppInit(int argc, char *const argv[]);
int AppDeinit();
int AppExit();
int AppExitToArch();
void AppLockExit();
void AppUnlockExit();

#endif