#ifndef __M_CONFIG_H__
#define __M_CONFIG_H__

#define CONFIG_VERSION 1

#define APP_DATA_DIR "ux0:data/EMU4VITA/" APP_DIR_NAME
#define APP_ASSETS_DIR "app0:assets"
#define CORE_DATA_DIR "app0:data"

#define FONT_PGF_PATH APP_ASSETS_DIR "/font.pgf"
#define WALLPAPER_PNG_PATH APP_ASSETS_DIR "/bg.png"
#define SPLASH_PNG_PATH APP_ASSETS_DIR "/splash.png"

#define APP_LOG_PATH APP_DATA_DIR "/log.txt"
#define CONFIG_PATH APP_DATA_DIR "/config.bin"
#define LASTFILE_PATH APP_DATA_DIR "/lastfile.txt"

#define REPOSITORY_ADDRESS "https://gitee.com/yizhigai/Emu4Vita"

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
    char reserved[0x10];   // 0x30
} Config;                  // 0x40

void trimString(char *str);
char *trimStringEx(char *str);

int configGetDecimal(const char *str);
int configGetHexdecimal(const char *str);
int configGetBoolean(const char *str);
char *configGetString(const char *str);

int configGetLine(const char *buf, int size, char **line);
int configReadLine(const char *line, char **name, char **string);

int ResetConfig();
int LoadConfig();
int SaveConfig();

/*
extern char *private_assets_dir;
extern char *public_assetss_dir;
*/
extern Config g_config;

#endif
