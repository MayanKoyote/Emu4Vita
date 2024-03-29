#ifndef __M_LANG_H__
#define __M_LANG_H__

#include "lang_types.h"

typedef struct
{
    unsigned int lang;
    char *string;
} LangString;

typedef struct
{
    char *name;
    char **container;
    int retro_lang;
} LangEntry;

extern char **cur_lang;

LangEntry *GetLangEntries();
int GetLangEntriesLength();
int GetLangIdBySystemLang(int local_lang);
int GetLangIdByConfigLang(int config_lang);
int GetConfigLangByLangId(int lang_id);
int GetRetroLangByLangId(int lang_id);
int SetCurrentLang(int lang_id);

char *GetLangString(LangString *lang_s);
char **GetStringArrayByLangArray(int *langs, int n_langs);

#endif