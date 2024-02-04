#ifndef __M_UTILS_STRING_H__
#define __M_UTILS_STRING_H__

#define IS_ENGLISH_CHARACTER(ch) ((ch >= 0x41 && ch <= 0x5A) || (ch >= 0x61 && ch <= 0x7A))

void TrimString(char *str);
char *TrimStringEx(char *str);

int StringToDecimal(const char *str);
int StringToHexdecimal(const char *str);
int StringToBoolean(const char *str);

int StringGetLine(const char *buf, int size, char **pline);
int StringReadConfigLine(const char *line, char **pkey, char **pvalue);

char *StringMakeShortByWidth(const char *string, int limit_w);
char *StringBreakLineByWidth(const char *string, int limit_w);

int StringToListByWidthEx(LinkedList *list, char *buffer, size_t size, int limit_width);
int StringToListByWidth(LinkedList *list, const char *str, int limit_width);

#endif
