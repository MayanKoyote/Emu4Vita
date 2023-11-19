#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "file.h"

/*
char *private_assets_dir = NULL;
char *public_assetss_dir = NULL;
*/
Config g_config;

void trimString(char *str)
{
    int len = strlen(str);
    int i;

    for (i = len - 1; i >= 0; i--)
    {
        if (str[i] == ' ' || str[i] == '\t')
        {
            str[i] = 0;
        }
        else
        {
            break;
        }
    }
}

char *trimStringEx(char *str)
{
    char *res = str;
    while (*res == ' ' || *res == '\t')
        res++;
    trimString(res);

    return res;
}

int configGetDecimal(const char *str)
{
    return strtol(str, NULL, 0);
}

int configGetHexdecimal(const char *str)
{
    return strtoul(str, NULL, 16);
}

int configGetBoolean(const char *str)
{
    if (strcasecmp(str, "false") == 0 ||
        strcasecmp(str, "off") == 0 ||
        strcasecmp(str, "no") == 0)
        return 0;

    if (strcasecmp(str, "true") == 0 ||
        strcasecmp(str, "on") == 0 ||
        strcasecmp(str, "yes") == 0)
        return 1;

    return -1;
}

char *configGetString(const char *str)
{
    if (str[0] != '"')
        return NULL;

    char *p = strchr(str + 1, '"');
    if (!p)
        return NULL;

    int len = p - (str + 1);

    char *out = malloc(len + 1);
    strncpy(out, str + 1, len);
    out[len] = '\0';

    int i;
    for (i = 0; i < len; i++)
    {
        if (out[i] == '\\')
            out[i] = '\n';
    }

    return out;
}

int configGetLine(const char *buf, int size, char **line)
{
    uint8_t ch = 0;
    int n = 0;
    int i = 0;
    uint8_t *pbuf = (uint8_t *)buf;

    for (i = 0; i < size; i++)
    {
        ch = pbuf[i];
        if (ch < 0x20 && ch != '\t')
        {
            i++;
            break;
        }
        n++;
    }
    char *_line = NULL;
    if (n > 0)
    {
        _line = malloc(n + 1);
        strncpy(_line, buf, n);
        _line[n] = '\0';
    }
    *line = _line;

    return i;
}

int configReadLine(const char *line, char **name, char **string)
{
    int ret = 1;
    int len;

    char *_line = NULL;
    char *_name = NULL;
    char *_string = NULL;

    len = strlen(line);
    _line = malloc(len + 1);
    if (!_line)
    {
        ret = -1;
        goto end;
    }
    strcpy(_line, line);

    // Trim at beginning
    char *_pline = _line;
    while (*_pline == ' ' || *_pline == '\t')
        _pline++;

    // Ignore comments #1
    if (_pline[0] == '#')
    {
        ret = 0;
        goto end;
    }

    // Ignore comments #2
    char *p = strchr(_pline, '#');
    if (p)
    {
        // APP_LOG("IGNORE %s\n", p);
        *p = '\0';
    }

    // Get token
    p = strchr(_pline, '=');
    if (!p)
    {
        ret = -1;
        goto end;
    }

    // Name
    len = p - _pline;
    _name = malloc(len + 1);
    if (_name)
    {
        strncpy(_name, _pline, len);
        _name[len] = '\0';
        trimString(_name);
    }
    // APP_LOG("NAME: %s\n", _name);

    p++;
    while (*p == ' ' || *p == '\t')
        p++;
    trimString(p);

    // String
    len = strlen(p);
    _string = malloc(len + 1);
    if (_string)
        strcpy(_string, p);
    // APP_LOG("STRING: %s\n", _string);
    ret = 1;

end:
    *name = _name;
    *string = _string;
    if (_line)
        free(_line);

    return ret;
}

int ResetConfig()
{
    memset(&g_config, 0, sizeof(Config));
    g_config.version = CONFIG_VERSION;

    return 0;
}

int LoadConfig()
{
    Config config;
    memset(&config, 0, sizeof(Config));

    int ret = ReadFile(CONFIG_PATH, &config, sizeof(Config));
    if (ret < 0 || ret != sizeof(Config) || config.version != CONFIG_VERSION)
    {
        ResetConfig();
        return -1;
    }

    memcpy(&g_config, &config, sizeof(Config));

    return 0;
}

int SaveConfig()
{
    char *parent_dir = getBaseDirectory(CONFIG_PATH);
    if (!parent_dir)
        return -1;
    createFolder(parent_dir);
    free(parent_dir);

    return WriteFile(CONFIG_PATH, &g_config, sizeof(Config));
}
