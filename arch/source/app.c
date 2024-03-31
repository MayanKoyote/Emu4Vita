#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/modulemgr.h>
#include <psp2/power.h>
#include <psp2/apputil.h>
#include <psp2/ctrl.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/system_param.h>
#include <psp2/appmgr.h>
#include <psp2/shellutil.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/devctl.h>
#include <psp2/vshbridge.h>
#include <psp2/common_dialog.h>

#include "app.h"
#include "utils.h"
#include "file.h"
#include "gui.h"
#include "config.h"

static int app_run = 0;
static int is_safe_mode = 0;
static int is_vitatv_model = 0;

int language = 0, enter_button = 0, date_format = 0, time_format = 0;

int IsSafeMode()
{
    return is_safe_mode;
}

int CheckSafeMode()
{
    if (sceIoDevctl("ux0:", 0x3001, NULL, 0, NULL, 0) == 0x80010030)
        is_safe_mode = 1;

    return is_safe_mode;
}

int IsVitatvModel()
{
    return is_vitatv_model;
}

int CheckVitatvModel()
{
    if (sceKernelGetModel() == SCE_KERNEL_MODEL_VITATV)
        is_vitatv_model = 1;

    return is_vitatv_model;
}

static void InitSceAppUtil()
{
    // Init SceAppUtil
    SceAppUtilInitParam init_param;
    SceAppUtilBootParam boot_param;
    memset(&init_param, 0, sizeof(SceAppUtilInitParam));
    memset(&boot_param, 0, sizeof(SceAppUtilBootParam));
    sceAppUtilInit(&init_param, &boot_param);

    // System params
    sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_LANG, &language);
    sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_ENTER_BUTTON, &enter_button);
    sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_DATE_FORMAT, &date_format);
    sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_TIME_FORMAT, &time_format);

    // Set common dialog config
    SceCommonDialogConfigParam config;
    sceCommonDialogConfigParamInit(&config);
    sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_LANG, (int *)&config.language);
    sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_ENTER_BUTTON, (int *)&config.enterButtonAssign);
    sceCommonDialogSetConfigParam(&config);
}

static void DeinitSceAppUtil()
{
    // Shutdown AppUtil
    sceAppUtilShutdown();
}

int AppStart()
{
    app_run = 1;

    while (app_run)
    {
        GUI_RunMain();
    }

    return 0;
}

int AppExit()
{
    app_run = 0;

    return 0;
}

int AppIsRunning()
{
    return app_run;
}

int AppInit()
{
    sceIoRemove(APP_LOG_PATH);
    CreateFolder(APP_DATA_DIR);

    scePowerSetArmClockFrequency(444);
    scePowerSetBusClockFrequency(222);
    scePowerSetGpuClockFrequency(222);
    scePowerSetGpuXbarClockFrequency(166);

    sceShellUtilInitEvents(0);
    InitSceAppUtil();

    sceCtrlSetSamplingModeExt(SCE_CTRL_MODE_ANALOG);

    LockUsbConnection();

    CheckVitatvModel();
    CheckSafeMode();

    GUI_Init();
    AppStart();

    return 0;
}

int AppDeinit()
{
    GUI_Deinit();
    DeinitSceAppUtil();

    return 0;
}
