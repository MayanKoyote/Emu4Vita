#ifndef __M_BROWSER_H__
#define __M_BROWSER_H__

#include <vita2d.h>

typedef struct
{
	float x, y;
	float w, h;
	float margin_r;
} IconLayout;

typedef struct
{
	char *desc;
	char *core_name;
	char *assets_name;
} CoreEntry;

typedef struct
{
	char *short_name;
	vita2d_texture *icon;
	CoreEntry *entries;
	int n_entries;
	uint32_t *entries_pos;
	IconLayout layout;
} SoftwareEntry;

int initBrowser();
int drawBrowser();
int ctrlBrowser();

#endif
