#ifndef __M_CONFIG_H__
#define __M_CONFIG_H__

#define CONFIG_VERSION 0x010900 // 1.09

#define REPOSITORY_ADDRESS "https://gitee.com/yizhigai/Emu4Vita"

#define APP_DATA_DIR "ux0:data/EMU4VITA/" APP_DIR_NAME
#define APP_ASSETS_DIR "app0:assets"
#define CORE_DATA_DIR "app0:data"

#define FONT_TTF_PATH APP_ASSETS_DIR "/font.ttf"
#define DEFAULT_WALLPAPER_PNG_PATH APP_ASSETS_DIR "/bg.png"

#define WALLPAPER_DIR APP_ASSETS_DIR "/wallpapers"
#define WALLPAPER_SAVEFILE_PATH APP_DATA_DIR "/lastwallpaper.txt"
#define APP_LOG_PATH APP_DATA_DIR "/log.txt"
#define CONFIG_PATH APP_DATA_DIR "/config.bin"

#define MAX_CONFIG_NAME_LENGTH 128
#define MAX_CONFIG_STRING_LENGTH 1024
#define MAX_CONFIG_LINE_LENGTH 2048

typedef struct
{
    uint32_t version;      // 0x00
    uint32_t software_pos; // 0x04
    uint32_t nes_pos;      // 0x08
    uint32_t snes_pos;     // 0x0C
    uint32_t md_pos;       // 0x10
    uint32_t gba_pos;      // 0x14
    uint32_t gbc_pos;      // 0x18
    uint32_t pce_pos;      // 0x1C
    uint32_t ps1_pos;      // 0x20
    uint32_t wsc_pos;      // 0x24
    uint32_t ngp_pos;      // 0x28
    uint32_t arc_pos;      // 0x2C
    uint32_t bg_num;       // 0x30
    char reserved[0x0C];   // 0x34
} Config;                  // 0x40

void TrimString(char *str);

int StringToDecimal(const char *str);
int StringToHexdecimal(const char *str);
int StringToBoolean(const char *str);

int GetConfigLine(const char *buf, int size, char **pline);
int ReadConfigLine(const char *line, char **pkey, char **pvalue);

int ResetConfig();
int LoadConfig();
int SaveConfig();

extern Config setting_config;

#endif
