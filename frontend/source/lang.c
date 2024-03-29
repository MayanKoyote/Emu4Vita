#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <psp2/system_param.h>
#include <libretro.h>

#include "lang.h"
#include "lang_intl.h"
#include "app.h"

enum LangId
{
    ID_LANG_ENGLISH,
    ID_LANG_JAPANESE,
    ID_LANG_FRENCH,
    ID_LANG_SPANISH,
    ID_LANG_GERMAN,
    ID_LANG_ITALIAN,
    ID_LANG_DUTCH,
    ID_LANG_PORTUGUESE_BRAZIL,
    ID_LANG_PORTUGUESE_PORTUGAL,
    ID_LANG_RUSSIAN,
    ID_LANG_KOREAN,
    ID_LANG_CHINESE_TRADITIONAL,
    ID_LANG_CHINESE_SIMPLIFIED,
    ID_LANG_ESPERANTO,
    ID_LANG_POLISH,
    ID_LANG_VIETNAMESE,
    ID_LANG_ARABIC,
    ID_LANG_GREEK,
    ID_LANG_TURKISH,
    ID_LANG_SLOVAK,
    ID_LANG_PERSIAN,
    ID_LANG_HEBREW,
    ID_LANG_ASTURIAN,
    ID_LANG_FINNISH,
};

static LangEntry lang_entries[] = {
    {"English", lang_us, RETRO_LANGUAGE_ENGLISH},
    {"Japanese", NULL, RETRO_LANGUAGE_JAPANESE},
    {"French", NULL, RETRO_LANGUAGE_FRENCH},
    {"Spanish", NULL, RETRO_LANGUAGE_SPANISH},
    {"German", NULL, RETRO_LANGUAGE_GERMAN},
    {"Italian", NULL, RETRO_LANGUAGE_ITALIAN},
    {"Dutch", NULL, RETRO_LANGUAGE_DUTCH},
    {"Portuguese (brazil)", NULL, RETRO_LANGUAGE_PORTUGUESE_BRAZIL},
    {"Portuguese (portugal)", NULL, RETRO_LANGUAGE_PORTUGUESE_PORTUGAL},
    {"Russian", NULL, RETRO_LANGUAGE_RUSSIAN},
    {"Korean", NULL, RETRO_LANGUAGE_KOREAN},
    {"繁體中文", lang_cht, RETRO_LANGUAGE_CHINESE_TRADITIONAL},
    {"简体中文", lang_chs, RETRO_LANGUAGE_CHINESE_SIMPLIFIED},
    {"Esperanto", NULL, RETRO_LANGUAGE_ESPERANTO},
    {"Polish", NULL, RETRO_LANGUAGE_POLISH},
    {"Vietnamese", NULL, RETRO_LANGUAGE_VIETNAMESE},
    {"Arabic", NULL, RETRO_LANGUAGE_ARABIC},
    {"Greek", NULL, RETRO_LANGUAGE_GREEK},
    {"Turkish", NULL, RETRO_LANGUAGE_TURKISH},
    {"Slovak", NULL, RETRO_LANGUAGE_SLOVAK},
    {"Persian", NULL, RETRO_LANGUAGE_PERSIAN},
    {"Hebrew", NULL, RETRO_LANGUAGE_HEBREW},
    {"Asturian", NULL, RETRO_LANGUAGE_ASTURIAN},
    {"Finnish", NULL, RETRO_LANGUAGE_FINNISH},
};
#define N_LANG_ENTRIES (sizeof(lang_entries) / sizeof(LangEntry))

char **cur_lang = lang_us;

LangEntry *GetLangEntries()
{
    return lang_entries;
}

int GetLangEntriesLength()
{
    return N_LANG_ENTRIES;
}

int GetLangIdBySystemLang(int system_lang)
{
    int ret = SCE_SYSTEM_PARAM_LANG_ENGLISH_US;

    switch (system_lang)
    {
    case SCE_SYSTEM_PARAM_LANG_JAPANESE:
        ret = ID_LANG_JAPANESE;
        break;
    case SCE_SYSTEM_PARAM_LANG_FRENCH:
        ret = ID_LANG_FRENCH;
        break;
    case SCE_SYSTEM_PARAM_LANG_SPANISH:
        ret = ID_LANG_SPANISH;
        break;
    case SCE_SYSTEM_PARAM_LANG_GERMAN:
        ret = ID_LANG_GERMAN;
        break;
    case SCE_SYSTEM_PARAM_LANG_ITALIAN:
        ret = ID_LANG_ITALIAN;
        break;
    case SCE_SYSTEM_PARAM_LANG_DUTCH:
        ret = ID_LANG_DUTCH;
        break;
    case SCE_SYSTEM_PARAM_LANG_PORTUGUESE_PT:
        ret = ID_LANG_PORTUGUESE_PORTUGAL;
        break;
    case SCE_SYSTEM_PARAM_LANG_RUSSIAN:
        ret = ID_LANG_RUSSIAN;
        break;
    case SCE_SYSTEM_PARAM_LANG_KOREAN:
        ret = ID_LANG_KOREAN;
        break;
    case SCE_SYSTEM_PARAM_LANG_CHINESE_T:
        ret = ID_LANG_CHINESE_TRADITIONAL;
        break;
    case SCE_SYSTEM_PARAM_LANG_CHINESE_S:
        ret = ID_LANG_CHINESE_SIMPLIFIED;
        break;
    case SCE_SYSTEM_PARAM_LANG_FINNISH:
        ret = ID_LANG_FINNISH;
        break;
    case SCE_SYSTEM_PARAM_LANG_POLISH:
        ret = ID_LANG_POLISH;
        break;
    case SCE_SYSTEM_PARAM_LANG_PORTUGUESE_BR:
        ret = ID_LANG_PORTUGUESE_BRAZIL;
        break;
    case SCE_SYSTEM_PARAM_LANG_TURKISH:
        ret = ID_LANG_TURKISH;
        break;
    case SCE_SYSTEM_PARAM_LANG_ENGLISH_US:
    case SCE_SYSTEM_PARAM_LANG_ENGLISH_GB:
    default:
        ret = ID_LANG_ENGLISH;
        break;
    }

    if (!lang_entries[ret].container)
        return ID_LANG_ENGLISH; // Use default us

    return ret;
}

int GetLangIdByConfigLang(int config_lang)
{
    int index = 0;
    int length = N_LANG_ENTRIES;

    int i;
    for (i = 0; i < length; i++)
    {
        if (lang_entries[i].container)
        {
            if (index == config_lang)
                break;
            else
                index++;
        }
    }
    if (i >= length)            // No found
        return ID_LANG_ENGLISH; // Use default us

    // printf("config_lang: %d ==> lang_id: %d\n", config_lang, i);
    return i;
}

int GetConfigLangByLangId(int lang_id)
{
    int config_lang = 0;
    int length = N_LANG_ENTRIES;

    int i;
    for (i = 0; i < length; i++)
    {
        if (i == lang_id)
            break;

        if (lang_entries[i].container)
            config_lang++;
    }
    if (i >= length) // No found
        return 0;    // Use default us

    // printf("lang_id: %d ==> config_lang: %d\n", lang_id, config_lang);
    return config_lang;
}

int GetRetroLangByLangId(int lang_id)
{
    if (lang_id < 0 || lang_id >= N_LANG_ENTRIES)
        return RETRO_LANGUAGE_ENGLISH;

    return lang_entries[lang_id].retro_lang;
}

int SetCurrentLang(int lang_id)
{
    int ret = 0;

    if (lang_id < 0 || lang_id >= N_LANG_ENTRIES || !lang_entries[lang_id].container)
    {
        ret = -1;
        goto END;
    }

    cur_lang = lang_entries[lang_id].container;

END:
    if (cur_lang)
    {
        if (system_enter_button == SCE_SYSTEM_PARAM_ENTER_BUTTON_CIRCLE)
        {
            cur_lang[LANG_LOCAL_BUTTON_ENTER] = cur_lang[LANG_LOCAL_BUTTON_B];
            cur_lang[LANG_LOCAL_BUTTON_CANCEL] = cur_lang[LANG_LOCAL_BUTTON_A];
        }
        else
        {
            cur_lang[LANG_LOCAL_BUTTON_ENTER] = cur_lang[LANG_LOCAL_BUTTON_A];
            cur_lang[LANG_LOCAL_BUTTON_CANCEL] = cur_lang[LANG_LOCAL_BUTTON_B];
        }
    }

    return ret;
}

char *GetLangString(LangString *lang_s)
{
    if (!lang_s)
        return NULL;

    char *res = NULL;
    // 如果有lang，返回lang
    if (lang_s->lang != LANG_NULL && lang_s->lang < LANGUAGE_CONTAINER_SIZE)
        res = cur_lang[lang_s->lang];
    // 如果没有lang，返回string
    if (res == NULL)
        res = lang_s->string;

    return res;
}

char **GetStringArrayByLangArray(int *langs, int n_langs)
{
    if (!langs || n_langs <= 0)
        return NULL;

    char **strs = calloc(n_langs, sizeof(char *));
    if (!strs)
        return NULL;

    int i;
    for (i = 0; i < n_langs; i++)
    {
        strs[i] = cur_lang[langs[i]];
    }

    return strs;
}
