#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "gui/gui.h"
#include "utils_string.h"
#include "utils.h"

void TrimString(char *str)
{
    int len = strlen(str);
    int i;

    for (i = len - 1; i >= 0; i--)
    {
        if (str[i] == ' ' || str[i] == '\t')
            str[i] = 0;
        else
            break;
    }
}

char *TrimStringEx(char *str)
{
    char *res = str;
    while (*res == ' ' || *res == '\t')
        res++;
    TrimString(res);

    return res;
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

int StringGetLine(const char *buf, int size, char **pline)
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

int StringReadConfigLine(const char *line, char **pkey, char **pvalue)
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

char *StringMakeShortByWidth(const char *string, int limit_w)
{
    if (!string)
        return NULL;

    char *res = NULL;
    int len = strlen(string);
    char *buf = (char *)malloc(len + 1);
    if (!buf)
        return NULL;
    strcpy(buf, string);

    int cut_w = limit_w - GUI_GetTextWidth("...");
    int cut = 0;
    int max_w = 0;
    char ch;
    int i, j, count;
    for (i = 0; i < len;)
    {
        if (buf[i] == '\n')
            break;

        count = GetUTF8Count(&buf[i]);
        j = i + count;
        if (j > len)
            break;
        ch = buf[j];
        buf[j] = '\0';
        max_w += GUI_GetTextWidth(&buf[i]);
        buf[j] = ch;

        if (max_w <= cut_w)
            cut = j;
        if (max_w > limit_w)
            break;

        i = j;
    }

    if (max_w > limit_w)
    {
        buf[cut] = '\0';
        if (cut + 4 > len)
        {
            res = (char *)malloc(cut + 4);
            sprintf(res, "%s...", buf);
            free(buf);
        }
        else
        {
            res = buf;
            strcat(res, "...");
        }
    }
    else
    {
        res = buf;
    }

    return res;
}

char *StringBreakLineByWidth(const char *string, int limit_w)
{
    if (!string)
        return NULL;

    int len = strlen(string) + 128;
    char *res = malloc(len + 1);
    if (!res)
        return NULL;
    strcpy(res, string);

    char ch;
    int w = 0, max_w = 0;
    int i, count;
    for (i = 0; i < len; i += count)
    {
        if (res[i] == '\n')
        {
            max_w = 0;
            count = 1;
            continue;
        }

        count = GetUTF8Count(&res[i]);
        if (i + count > len)
        {
            res[i] = '\0';
            break;
        }

        ch = res[i + count];
        res[i + count] = '\0';
        w = GUI_GetTextWidth(&res[i]);
        res[i + count] = ch;

        if (max_w + w >= limit_w)
        {
            max_w = w;
            memmove(&res[i + 1], &res[i], len - i - 1);
            res[i++] = '\n';
        }
        else
        {
            max_w += w;
        }
    }

    return res;
}

int StringToListByWidthEx(LinkedList *list, char *buffer, size_t size, int limit_width)
{
    if (!list || !buffer)
        return 0;

    char *string;
    char *start = buffer;
    char *finish = buffer + size;
    char *last_space_p = start;
    char *p = start;
    int width = 0;
    int max_width = 0;
    int last_space_w = 0;
    int count;
    int ch_w;
    char ch;
    while (p < finish)
    {
        count = GetUTF8Count(p);
        ch = *(p + count);
        *(p + count) = '\0';
        ch_w = GUI_GetTextWidth(p);
        *(p + count) = ch;

        if (*p == ' ' || *p == '\t')
        {
            last_space_p = p;
            last_space_w = width;
        }

        if (*p == '\n' || width + ch_w > limit_width)
        {
            // Check english word truncated (if current character and space is not in the line's first)
            if ((p > start && last_space_p > start) && (IS_ENGLISH_CHARACTER(*p) && IS_ENGLISH_CHARACTER(*(p - 1))))
            {
                // Go back to the last space, current word will be in the next line
                p = last_space_p + 1;
                count = 0;            // Set to zero for skip auto step
                ch_w = 0;             // Set to zero for skip auto step
                width = last_space_w; // Set to last space width
                last_space_w = 0;
            }
            ch = *p;
            *p = '\0';
            string = malloc(strlen(start) + 1);
            if (string)
            {
                strcpy(string, start);
                LinkedListAdd(list, string);
            }
            *p = ch;
            if (*p == '\n')
                p++;
            start = p;
            if (width > max_width)
                max_width = width;
            width = ch_w;
        }
        else
        {
            width += ch_w;
        }
        p += count;
    }
    if (start < finish)
    {
        string = malloc(strlen(start) + 1);
        if (string)
        {
            strcpy(string, start);
            LinkedListAdd(list, string);
        }
    }

    if (width > max_width)
        max_width = width;

    return max_width;
}

int StringToListByWidth(LinkedList *list, const char *str, int limit_width)
{
    if (!list || !str)
        return 0;

    int len = strlen(str);
    char *buf = malloc(len + 1);
    if (!buf)
        return 0;
    strcpy(buf, str);

    int ret = StringToListByWidthEx(list, buf, len, limit_width);
    free(buf);

    return ret;
}
