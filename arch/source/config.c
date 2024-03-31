#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "file.h"

Config setting_config;

void TrimString(char *str)
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

int StringToDecimal(const char *str)
{
    return strtol(str, NULL, 0);
}

int StringToHexdecimal(const char *str)
{
    return strtoul(str, NULL, 16);
}

int StringToBoolean(const char *str)
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

int GetConfigLine(const char *buf, int size, char **pline)
{
    int n = 0;
    int i = 0;
    uint8_t *p = (uint8_t *)buf;

    for (i = 0; i < size; i++)
    {
        if (p[i] < 0x20 && p[i] != '\t')
        {
            i++;
            break;
        }
        n++;
    }

    if (pline && n > 0)
    {
        char *line = malloc(n + 1);
        if (line)
        {
            strncpy(line, buf, n);
            line[n] = '\0';
            *pline = line;
        }
    }

    return i;
}

int ReadConfigLine(const char *line, char **pkey, char **pvalue)
{
    if (!line)
        return -1;

    char *key = NULL;
    char *value = NULL;
    const char *head = line;
    const char *tail = line + strlen(line);
    int len;

    // Trim at beginning
    while (*head == ' ' || *head == '\t')
        head++;

    // Ignore comments #1
    if (head[0] == '#')
    {
        // printf("IGNORE %s\n", line);
        goto FAILED;
    }

    // Ignore comments #2
    const char *p = strchr(head, '#');
    if (p)
    {
        // printf("IGNORE %s\n", p);
        tail = p;
    }

    // Get token
    p = strchr(head, '=');
    if (!p || p >= tail)
        goto FAILED;

    // Key
    len = p - head;
    key = malloc(len + 1);
    if (!key)
        goto FAILED;
    strncpy(key, head, len);
    key[len] = '\0';
    TrimString(key);
    // printf("KEY: %s\n", key);

    head = p + 1;
    while (*head == ' ' || *head == '\t')
        head++;

    if (head[0] == '"') // String
    {
        head++;
        p = strchr(head, '"');
        if (!p || p >= tail)
            goto FAILED;
    }
    else // Decimal
    {
        p = tail;
        while (p > head && (*(p - 1) == ' ' || *(p - 1) == '\t'))
            p--;
    }

    // Value
    len = p - head;
    value = malloc(len + 1);
    if (!value)
        goto FAILED;
    strncpy(value, head, len);
    value[len] = '\0';
    // printf("VALUE: %s\n", value);

    *pkey = key;
    *pvalue = value;
    return 0;

FAILED:
    if (key)
        free(key);
    if (value)
        free(value);
    return -1;
}

int ResetConfig()
{
    memset(&setting_config, 0, sizeof(Config));
    setting_config.version = CONFIG_VERSION;

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

    memcpy(&setting_config, &config, sizeof(Config));
    return 0;
}

int SaveConfig()
{
    char *parent_dir = GetBaseDirectory(CONFIG_PATH);
    if (!parent_dir)
        return -1;

    CreateFolder(parent_dir);
    free(parent_dir);

    return WriteFile(CONFIG_PATH, &setting_config, sizeof(Config));
}
