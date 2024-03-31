#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

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
#include <psp2/touch.h>
#include <psp2/common_dialog.h>

#include "activity/activity.h"
#include "setting/setting.h"
#include "gui/gui.h"
#include "emu/emu.h"
#include "app.h"
#include "utils.h"
#include "file.h"
#include "config.h"
#include "boot.h"
#include "lang.h"

typedef enum AppExitEventAction
{
    APP_EXIT_EVENT_ACTION_NONE,
    APP_EXIT_EVENT_ACTION_BACK_TO_ARCH,
} AppExitEventAction;

#ifdef SCE_LIBC_SIZE
unsigned int sceLibcHeapSize = SCE_LIBC_SIZE;
#endif

static int app_run = 0;
static AppExitEventAction app_exit_event_action = APP_EXIT_EVENT_ACTION_NONE;
static int is_safe_mode = 0;
static int is_vitatv_model = 0;

int system_language = 0, system_enter_button = 0, system_date_format = 0, system_time_format = 0;

int CheckSafeMode()
{
    is_safe_mode = (sceIoDevctl("ux0:", 0x3001, NULL, 0, NULL, 0) == 0x80010030);
    return is_safe_mode;
}

int CheckVitatvModel()
{
    is_vitatv_model = (sceKernelGetModel() == SCE_KERNEL_MODEL_VITATV);
    return is_vitatv_model;
}

int IsSafeMode()
{
    return is_safe_mode;
}

int IsVitatvModel()
{
    return is_vitatv_model;
}

static int onSafeModeAlertDialogNegativeClick(AlertDialog *dialog, int which)
{
    AppExit();
    AlertDialog_Dismiss(dialog);
    return 0;
}

static void showSafeModeDialog()
{
    AlertDialog *tip_dialog = AlertDialog_Create();
    AlertDialog_SetTitle(tip_dialog, cur_lang[LANG_TIP]);
    AlertDialog_SetMessage(tip_dialog, cur_lang[LANG_MESSAGE_WARN_SAFE_MODE]);
    AlertDialog_SetNegativeButton(tip_dialog, cur_lang[LANG_EXIT], onSafeModeAlertDialogNegativeClick);
    AlertDialog_Show(tip_dialog);
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
    sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_LANG, &system_language);
    sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_ENTER_BUTTON, &system_enter_button);
    sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_DATE_FORMAT, &system_date_format);
    sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_TIME_FORMAT, &system_time_format);

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

    while (app_run || Emu_IsGameLoaded()) // 游戏已加载的话，需要等待游戏结束后再结束程序
    {
        GUI_RunMain();
    }

    return 0;
}

int AppExit()
{
    GUI_FinishOtherActivities();
    app_run = 0;
    return 0;
}

int AppExitToArch()
{
    app_exit_event_action = APP_EXIT_EVENT_ACTION_BACK_TO_ARCH;
    AppExit();
    return 0;
}

int AppEventExit()
{
    if (app_exit_event_action == APP_EXIT_EVENT_ACTION_BACK_TO_ARCH)
        return BootRestoreApp();

    return 0;
}

int AppInit(int argc, char *const argv[])
{
    sceIoRemove(APP_LOG_PATH);
    sceIoRemove(CORE_LOG_PATH);
    CreateFolder(APP_DATA_DIR);
    CreateFolder(CORE_CHEATS_DIR);
    CreateFolder(CORE_SPLASHS_DIR);

    scePowerSetArmClockFrequency(444);
    scePowerSetBusClockFrequency(222);
    scePowerSetGpuClockFrequency(222);
    scePowerSetGpuXbarClockFrequency(166);

    // Init sceShellUtil events
    sceShellUtilInitEvents(0);

    // Init sceAppUtil
    InitSceAppUtil();

    sceCtrlSetSamplingModeExt(SCE_CTRL_MODE_ANALOG);
    sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);
    sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK, SCE_TOUCH_SAMPLING_STATE_START);

    LoadAppConfig(TYPE_CONFIG_MAIN);
    LoadControlConfig(TYPE_CONFIG_MAIN);
    LoadHotkeyConfig(TYPE_CONFIG_MAIN);
    LoadMiscConfig(TYPE_CONFIG_MAIN);
    LoadGraphicsConfig(TYPE_CONFIG_MAIN);

    APP_LOG("[APP] App init...\n");

    SetCurrentLang(app_config.language);

    BootCheckParams(argc, argv);
    CheckVitatvModel();
    CheckSafeMode();

    GUI_Init();

    if (is_safe_mode)
    {
        showSafeModeDialog();
        goto END;
    }

    Emu_Init();
    Setting_Init();

#if defined(WANT_EXT_ARCHIVE_ROM)
    Archive_LoadCacheConfig();
#endif

    // Lock USB connection and PS button
    sceShellUtilLock(SCE_SHELL_UTIL_LOCK_TYPE_USB_CONNECTION | SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN_2);

    // GUI_WaitInitEnd();
    GUI_StartActivity(&browser_activity);

    APP_LOG("[APP] App init OK!\n");

    if (BootGetMode() == BOOT_MODE_GAME)
        BootLoadGame();

END:
    AppStart();

    return 0;
}

int AppDeinit()
{
    APP_LOG("[APP] App deinit...\n");

    AppEventExit();
    Setting_Deinit();
    Emu_Deinit();
    GUI_Deinit();
    DeinitSceAppUtil();

    APP_LOG("[APP] App deinit OK!\n");

    return 0;
}
