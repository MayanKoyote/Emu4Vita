#ifndef __M_CONFIG_H__
#define __M_CONFIG_H__

#include "list/config_list.h"
#include "list/option_list.h"
#include "list/overlay_list.h"
#include "list/cheat_list.h"
#include "config_types.h"
#include "config_values.h"
#include "utils_string.h"
#include "file.h"
#include "path.h"

extern AppConfig app_config;
extern GraphicsConfig graphics_config;
extern ControlConfig control_config;
extern HotkeyConfig hotkey_config;
extern MiscConfig misc_config;

extern char *private_assets_dir;
extern char *public_assets_dir;

extern LinkedList *core_cheat_list;
extern LinkedList *core_option_list;
extern LinkedList *graphics_overlay_list;

int MakeConfigPath(char *path, char *config_name, int type);

int ResetGraphicsConfig();
int ResetControlConfig();
int ResetHotkeyConfig();
int ResetMiscConfig();
int ResetAppConfig();

int LoadGraphicsConfig(int type);
int LoadControlConfig(int type);
int LoadHotkeyConfig(int type);
int LoadMiscConfig(int type);
int LoadAppConfig(int type);

int SaveGraphicsConfig(int type);
int SaveControlConfig(int type);
int SaveHotkeyConfig(int type);
int SaveMiscConfig(int type);
int SaveAppConfig(int type);

int ResetCoreConfig();
int LoadCoreConfig(int type);
int SaveCoreConfig(int type);

#endif
