#ifndef __M_APP_H__
#define __M_APP_H__

int AppInit();
int AppDeinit();

int AppIsRunning();

int IsSafeMode();
int IsVitatvModel();

extern int language, enter_button, date_format, time_format;

#endif