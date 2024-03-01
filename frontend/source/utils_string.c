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

    int len = strlen(string);
    char *res = (char *)malloc(len + 4);
    if (!res)
        return NULL;
    strcpy(res, string);

    int cut_w = limit_w - GUI_GetTextWidth("...");
    int cut = 0;
    int max_w = 0;
    char ch;
    int i, count;
    for (i = 0; i < len; i += count)
    {
        if (res[i] == '\n')
        {
            res[i] = '\0';
            break;
        }

        count = GetUTF8Count(&res[i]);
        if (i + count > len)
        {
            res[i] = '\0';
            break;
        }

        ch = res[i + count];
        res[i + count] = '\0';
        max_w += GUI_GetTextWidth(&res[i]);
        res[i + count] = ch;

        if (max_w <= cut_w)
            cut = i + count;
        if (max_w > limit_w)
            break;
    }

    if (max_w > limit_w)
    {
        res[cut] = '\0';
        strcat(res, "...");
    }

    return res;
}

char *StringBreakLineByWidth(const char *string, int limit_w)
{
    if (!string)
        return NULL;

    int len = strlen(string);
    int new_len = len + 128; // 预留128个字符以添加\n
    char *res = malloc(new_len + 1);
    if (!res)
        return NULL;
    strcpy(res, string);

    char ch;
    int w = 0, max_w = 0;
    int move_len = 0;
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
        if (i + count > len) // 异常字符停止
        {
            res[i] = '\0';
            break;
        }

        ch = res[i + count];
        res[i + count] = '\0';
        w = GUI_GetTextWidth(&res[i]);
        res[i + count] = ch;

        if (max_w + w > limit_w)
        {
            // 需留出一个字符添加\n，对应下面的len++不会超过new_len的长度
            if (len >= new_len)
            {
                len = new_len - 1;
                res[len] = '\0';
            }

            // 往后移动一个字符以添加\n
            move_len = len - i + 1; // +1包含\0
            memmove(&res[i + 1], &res[i], move_len);
            res[i] = '\n';
            i++;   // 前移跳过新添加的\n
            len++; // 因添加了\n长度增长
            max_w = w;
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
    char *cut = start;
    int line_w = 0;
    int max_w = 0;
    int last_space_w = 0;
    int w;
    int count;
    char ch;
    while (p < finish)
    {
        count = GetUTF8Count(p);
        ch = *(p + count);
        *(p + count) = '\0';
        w = GUI_GetTextWidth(p);
        *(p + count) = ch;

        if (*p == ' ' || *p == '\t')
        {
            last_space_p = p;
            last_space_w = line_w + w;
        }

        if (*p == '\n' || line_w + w > limit_width)
        {
            cut = p;
            
            // Check english word truncated
            if ((p > start && last_space_p > start) && (IS_ENGLISH_CHARACTER(*p) && IS_ENGLISH_CHARACTER(*(p - 1))))
            {
                cut = last_space_p + 1; // Go back to the last space, current word will be in the next line
                w = line_w + w - last_space_w;
                line_w = last_space_w;
                last_space_w = 0;
            }

            ch = *cut;
            *cut = '\0';
            string = malloc(strlen(start) + 1);
            if (string)
            {
                strcpy(string, start);
                LinkedListAdd(list, string);
            }
            *cut = ch;

            if (line_w > max_w)
                max_w = line_w;

            if (*cut == '\n')
            {
                start = cut + 1;
                line_w = 0;
            }
            else
            {
                start = cut;
                line_w = w;
            }
        }
        else
        {
            line_w += w;
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

    if (line_w > max_w)
        max_w = line_w;

    return max_w;
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
