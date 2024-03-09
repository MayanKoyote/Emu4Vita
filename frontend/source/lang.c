#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <psp2/system_param.h>
#include <libretro.h>

#include "lang.h"
#include "lang_intl.h"
#include "init.h"

enum LangIndex
{
    LANG_INDEX_ENGLISH,
    LANG_INDEX_JAPANESE,
    LANG_INDEX_FRENCH,
    LANG_INDEX_SPANISH,
    LANG_INDEX_GERMAN,
    LANG_INDEX_ITALIAN,
    LANG_INDEX_DUTCH,
    LANG_INDEX_PORTUGUESE_BRAZIL,
    LANG_INDEX_PORTUGUESE_PORTUGAL,
    LANG_INDEX_RUSSIAN,
    LANG_INDEX_KOREAN,
    LANG_INDEX_CHINESE_TRADITIONAL,
    LANG_INDEX_CHINESE_SIMPLIFIED,
    LANG_INDEX_ESPERANTO,
    LANG_INDEX_POLISH,
    LANG_INDEX_VIETNAMESE,
    LANG_INDEX_ARABIC,
    LANG_INDEX_GREEK,
    LANG_INDEX_TURKISH,
    LANG_INDEX_SLOVAK,
    LANG_INDEX_PERSIAN,
    LANG_INDEX_HEBREW,
    LANG_INDEX_ASTURIAN,
    LANG_INDEX_FINNISH,
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

#define N_LANGS (sizeof(lang_entries) / sizeof(LangEntry))

char **cur_lang = lang_us;

LangEntry *GetLangEntries()
{
    return lang_entries;
}

int GetLangEntriesLength()
{
    return N_LANGS;
}

int GetLangIndexByLocalLang(int local_lang)
{
    int ret = SCE_SYSTEM_PARAM_LANG_ENGLISH_US;

    switch (local_lang)
    {
    case SCE_SYSTEM_PARAM_LANG_JAPANESE:
        ret = LANG_INDEX_JAPANESE;
        break;
    case SCE_SYSTEM_PARAM_LANG_FRENCH:
        ret = LANG_INDEX_FRENCH;
        break;
    case SCE_SYSTEM_PARAM_LANG_SPANISH:
        ret = LANG_INDEX_SPANISH;
        break;
    case SCE_SYSTEM_PARAM_LANG_GERMAN:
        ret = LANG_INDEX_GERMAN;
        break;
    case SCE_SYSTEM_PARAM_LANG_ITALIAN:
        ret = LANG_INDEX_ITALIAN;
        break;
    case SCE_SYSTEM_PARAM_LANG_DUTCH:
        ret = LANG_INDEX_DUTCH;
        break;
    case SCE_SYSTEM_PARAM_LANG_PORTUGUESE_PT:
        ret = LANG_INDEX_PORTUGUESE_PORTUGAL;
        break;
    case SCE_SYSTEM_PARAM_LANG_RUSSIAN:
        ret = LANG_INDEX_RUSSIAN;
        break;
    case SCE_SYSTEM_PARAM_LANG_KOREAN:
        ret = LANG_INDEX_KOREAN;
        break;
    case SCE_SYSTEM_PARAM_LANG_CHINESE_T:
        ret = LANG_INDEX_CHINESE_TRADITIONAL;
        break;
    case SCE_SYSTEM_PARAM_LANG_CHINESE_S:
        ret = LANG_INDEX_CHINESE_SIMPLIFIED;
        break;
    case SCE_SYSTEM_PARAM_LANG_FINNISH:
        ret = LANG_INDEX_FINNISH;
        break;
    case SCE_SYSTEM_PARAM_LANG_POLISH:
        ret = LANG_INDEX_POLISH;
        break;
    case SCE_SYSTEM_PARAM_LANG_PORTUGUESE_BR:
        ret = LANG_INDEX_PORTUGUESE_BRAZIL;
        break;
    case SCE_SYSTEM_PARAM_LANG_TURKISH:
        ret = LANG_INDEX_TURKISH;
        break;
    case SCE_SYSTEM_PARAM_LANG_ENGLISH_US:
    case SCE_SYSTEM_PARAM_LANG_ENGLISH_GB:
    default:
        ret = LANG_INDEX_ENGLISH;
        break;
    }

    if (!lang_entries[ret].container)
        return LANG_INDEX_ENGLISH; // Use default us

    return ret;
}

int GetLangIndexByConfigValue(int config_value)
{
    int index = 0;
    int length = N_LANGS;

    int i;
    for (i = 0; i < length; i++)
    {
        if (lang_entries[i].container)
        {
            if (index == config_value)
                break;
            else
                index++;
        }
    }
    if (i >= length)               // No finded
        return LANG_INDEX_ENGLISH; // Use default us

    // printf("config_value: %d ==> lang_index: %d\n", config_value, i);
    return i;
}

int GetConfigValueByLangIndex(int lang_index)
{
    int config_value = 0;
    int length = N_LANGS;

    int i;
    for (i = 0; i < length; i++)
    {
        if (i == lang_index)
            break;

        if (lang_entries[i].container)
            config_value++;
    }
    if (i >= length) // No finded
        return 0;    // Use default us

    // printf("lang_index: %d ==> config_value: %d\n", lang_index, config_value);
    return config_value;
}

int GetRetroLangByLangIndex(int lang_index)
{
    if (lang_index < 0 || lang_index >= N_LANGS)
        return RETRO_LANGUAGE_ENGLISH;

    return lang_entries[lang_index].retro_lang;
}

int SetCurrentLang(int lang_index)
{
    int ret = 0;

    if (lang_index < 0 || lang_index >= N_LANGS || !lang_entries[lang_index].container)
    {
        ret = -1;
        goto END;
    }

    cur_lang = lang_entries[lang_index].container;

END:
    if (cur_lang)
    {
        if (enter_button == SCE_SYSTEM_PARAM_ENTER_BUTTON_CIRCLE)
        {
            cur_lang[LANG_BUTTON_ENTER] = cur_lang[LANG_BUTTON_CIRCLE];
            cur_lang[LANG_BUTTON_CANCEL] = cur_lang[LANG_BUTTON_CROSS];
        }
        else
        {
            cur_lang[LANG_BUTTON_ENTER] = cur_lang[LANG_BUTTON_CROSS];
            cur_lang[LANG_BUTTON_CANCEL] = cur_lang[LANG_BUTTON_CIRCLE];
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
