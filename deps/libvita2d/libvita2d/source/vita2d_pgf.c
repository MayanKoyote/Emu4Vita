#include <psp2/pgf.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/kernel/threadmgr.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <malloc.h>
#include <math.h>
#include "vita2d.h"
#include "texture_atlas.h"
#include "utils.h"
#include "shared.h"

#define ATLAS_DEFAULT_W 512
#define ATLAS_DEFAULT_H 512

#define FONT_GLYPH_MARGIN 2

typedef struct vita2d_pgf_font_handle {
	SceFontHandle font_handle;
	int (*in_font_group)(unsigned int c);
	struct vita2d_pgf_font_handle *next;
} vita2d_pgf_font_handle;

typedef struct vita2d_pgf {
	SceFontLibHandle lib_handle;
	vita2d_pgf_font_handle *font_handle_list;
	texture_atlas *atlas;
	SceKernelLwMutexWork mutex;
	int font_size;
	int max_height;
	int max_ascender;
	int max_descender;
	int line_space;
} vita2d_pgf;

static void *pgf_alloc_func(void *userdata, unsigned int size)
{
	return memalign(sizeof(int), (size + sizeof(int) - 1) / sizeof(int) * sizeof(int) );
}

static void pgf_free_func(void *userdata, void *p)
{
	free(p);
}

static int vita2d_load_pgf_post(vita2d_pgf *font) {
	SceFontInfo fontinfo;

	sceFontGetFontInfo(font->font_handle_list->font_handle, &fontinfo);
	font->font_size = fontinfo.maxGlyphHeight;
	font->max_height = fontinfo.maxGlyphHeightF + 0.5f;
	font->max_ascender = fontinfo.maxGlyphAscenderF + 0.5f;
	font->max_descender = fontinfo.maxGlyphDescenderF + 0.5f;
	// printf("[VITA2D_PGF] font->font_size: %d, font->max_height: %d, font->max_ascender = %d, font->max_descender = %d\n",
	//		font->font_size, font->max_height, font->max_ascender, font->max_descender);

	font->atlas = texture_atlas_create(ATLAS_DEFAULT_W, ATLAS_DEFAULT_H,
		SCE_GXM_TEXTURE_FORMAT_U8_R111);

	if (!font->atlas)
		return 0;

	sceKernelCreateLwMutex(&font->mutex, "vita2d_pgf_mutex", 2, 0, NULL);
	return 1;
}

static vita2d_pgf *vita2d_load_pgf_pre(int numFonts)
{
	unsigned int error;
	vita2d_pgf *font = malloc(sizeof(*font));
	if (!font)
		return NULL;
	memset(font, 0, sizeof(vita2d_pgf));

	SceFontNewLibParams params = {
		font,
		numFonts,
		NULL,
		pgf_alloc_func,
		pgf_free_func,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	};

	font->lib_handle = sceFontNewLib(&params, &error);
	if (error != 0) {
		free(font);
		return NULL;
	}
	return font;
}

vita2d_pgf *vita2d_load_system_pgf(int numFonts, const vita2d_system_pgf_config *configs)
{
	if (numFonts < 1) {
		return NULL;
	}

	unsigned int error;
	int i;

	vita2d_pgf *font = vita2d_load_pgf_pre(numFonts);

	if (!font)
		return NULL;

	SceFontStyle style = {0};
	style.fontH = 10;
	style.fontV = 10;

	vita2d_pgf_font_handle *tmp = NULL;

	for (i = 0; i < numFonts; i++) {
		style.fontLanguage = configs[i].code;
		int index = sceFontFindOptimumFont(font->lib_handle, &style, &error);
		if (error != 0)
			goto cleanup;

		SceFontHandle handle = sceFontOpen(font->lib_handle, index, 0, &error);
		if (error != 0)
			goto cleanup;

		if (font->font_handle_list == NULL) {
			tmp = font->font_handle_list = malloc(sizeof(vita2d_pgf_font_handle));
		} else {
			tmp = tmp->next = malloc(sizeof(vita2d_pgf_font_handle));
		}
		if (!tmp) {
			sceFontClose(handle);
			goto cleanup;
		}

		memset(tmp, 0, sizeof(vita2d_pgf_font_handle));
		tmp->font_handle = handle;
		tmp->in_font_group = configs[i].in_font_group;
	}

	if (!vita2d_load_pgf_post(font)) {
		vita2d_free_pgf(font);
		return NULL;
	}

	return font;

cleanup:
	tmp = font->font_handle_list;
	while (tmp) {
		vita2d_pgf_font_handle *next = tmp->next;
		sceFontClose(tmp->font_handle);
		free(tmp);
		tmp = next;
	}
	sceFontDoneLib(font->lib_handle);
	free(font);
	return NULL;
}

vita2d_pgf *vita2d_load_default_pgf()
{
	vita2d_system_pgf_config configs[] = {
		{SCE_FONT_LANGUAGE_DEFAULT, NULL},
	};

	return vita2d_load_system_pgf(1, configs);
}

vita2d_pgf *vita2d_load_custom_pgf(const char *path)
{
	unsigned int error;
	vita2d_pgf *font = vita2d_load_pgf_pre(1);

	if (!font)
		return NULL;

	vita2d_pgf_font_handle *handle = malloc(sizeof(vita2d_pgf_font_handle));
	if (!handle) {
		free(font);
		return NULL;
	}

	SceFontHandle font_handle = sceFontOpenUserFile(font->lib_handle, (char *)path, 1, &error);
	if (error != 0) {
		sceFontDoneLib(font->lib_handle);
		free(handle);
		free(font);
		return NULL;
	}
	memset(handle, 0, sizeof(vita2d_pgf_font_handle));
	handle->font_handle = font_handle;
	font->font_handle_list = handle;

	if (!vita2d_load_pgf_post(font)) {
		vita2d_free_pgf(font);
		return NULL;
	}

	return font;
}

void vita2d_free_pgf(vita2d_pgf *font)
{
	if (font) {
		sceKernelDeleteLwMutex(&font->mutex);

		vita2d_pgf_font_handle *tmp = font->font_handle_list;
		while (tmp) {
			vita2d_pgf_font_handle *next = tmp->next;
			sceFontClose(tmp->font_handle);
			free(tmp);
			tmp = next;
		}
		sceFontDoneLib(font->lib_handle);
		if (font->atlas)
			texture_atlas_free(font->atlas);
		free(font);
	}
}

static int atlas_add_glyph(vita2d_pgf *font, unsigned int character)
{
	SceFontHandle font_handle = font->font_handle_list->font_handle;
	SceFontCharInfo char_info;
	vita2d_position position;
	void *texture_data;
	vita2d_texture *tex = NULL;

	vita2d_pgf_font_handle *tmp = font->font_handle_list;
	while (tmp) {
		if (tmp->in_font_group == NULL || tmp->in_font_group(character)) {
			font_handle = tmp->font_handle;
			break;
		}
		tmp = tmp->next;
	}

	if (sceFontGetCharInfo(font_handle, character, &char_info) < 0)
		return 0;

	vita2d_size size = {
		char_info.bitmapWidth + FONT_GLYPH_MARGIN * 2,
		char_info.bitmapHeight + FONT_GLYPH_MARGIN * 2
	};

	texture_atlas_entry_data data = {
		char_info.bitmapLeft,
		char_info.bitmapTop,
		char_info.sfp26AdvanceH >> 6,
		char_info.sfp26AdvanceV >> 6,
		font->font_size
	};

	if (!texture_atlas_insert(font->atlas, character, &size, &data,
				  &tex, &position))
			return 0;

	texture_data = vita2d_texture_get_datap(tex);

	SceFontGlyphImage glyph_image;
	glyph_image.pixelFormat = SCE_FONT_PIXELFORMAT_8;
	glyph_image.xPos64 = (position.x + FONT_GLYPH_MARGIN) << 6;
	glyph_image.yPos64 = (position.y + FONT_GLYPH_MARGIN) << 6;
	glyph_image.bufWidth = vita2d_texture_get_width(tex);
	glyph_image.bufHeight = vita2d_texture_get_height(tex);
	glyph_image.bytesPerLine = vita2d_texture_get_stride(tex);
	glyph_image.pad = 0;
	glyph_image.bufferPtr = (unsigned int)texture_data;

	return sceFontGetCharGlyphImage(font_handle, character, &glyph_image) == 0;
}

static int generic_pgf_draw_text(vita2d_pgf *font, int draw, int *height,
									int x, int y, unsigned int color,
									unsigned int size,
									const char *text)
{
	sceKernelLockLwMutex(&font->mutex, 1, NULL);

	int i;
	unsigned int character;
	vita2d_rectangle rect;
	texture_atlas_entry_data data;
	vita2d_texture *tex = NULL;
	float scale;
	int start_x = x;
	int max_x = 0;
	int pen_x = x;
	int pen_y = y;
	int line_height = vita2d_pgf_get_lineheight(font, size);

	for (i = 0; text[i];) {
		i += utf8_to_ucs2(&text[i], &character);

		if (character == '\n') {
			if (pen_x > max_x)
				max_x = pen_x;
			pen_x = start_x;
			pen_y += (line_height + font->line_space);
			continue;
		}

		if (!texture_atlas_get(font->atlas, character, &tex, &rect, &data)) {
			if (!atlas_add_glyph(font, character))
				continue;

			if (!texture_atlas_get(font->atlas, character, &tex, &rect, &data))
					continue;
		}

		scale = (float)size / (float)data.glyph_size;

		if (draw) {
			vita2d_draw_texture_tint_part_scale(tex,
				pen_x + data.bitmap_left * scale, 
				pen_y + (font->max_ascender - data.bitmap_top) * scale,
				rect.x + FONT_GLYPH_MARGIN / 2.0f, rect.y + FONT_GLYPH_MARGIN / 2.0f,
				rect.w - FONT_GLYPH_MARGIN / 2.0f, rect.h - FONT_GLYPH_MARGIN / 2.0f,
				scale,
				scale,
				color);
		}

		pen_x += data.advance_x * scale;
	}

	if (pen_x > max_x)
		max_x = pen_x;

	if (height)
		*height = pen_y + size - y;

	sceKernelUnlockLwMutex(&font->mutex, 1);

	return max_x - x;
}

int vita2d_pgf_draw_text(vita2d_pgf *font, int x, int y,
			 unsigned int color, unsigned int size,
			 const char *text)
{
	return generic_pgf_draw_text(font, 1, NULL, x, y, color, size, text);
}

int vita2d_pgf_draw_textf(vita2d_pgf *font, int x, int y,
			  unsigned int color, unsigned int size,
			  const char *text, ...)
{
	char buf[1024];
	va_list argptr;
	va_start(argptr, text);
	vsnprintf(buf, sizeof(buf), text, argptr);
	va_end(argptr);
	return vita2d_pgf_draw_text(font, x, y, color, size, buf);
}

void vita2d_pgf_text_dimensions(vita2d_pgf *font, unsigned int size,
				const char *text, int *width, int *height)
{
	int w;
	w = generic_pgf_draw_text(font, 0, height, 0, 0, 0, size, text);

	if (width)
		*width = w;
}

int vita2d_pgf_text_width(vita2d_pgf *font, unsigned int size, const char *text)
{
	int width;
	vita2d_pgf_text_dimensions(font, size, text, &width, NULL);
	return width;
}

int vita2d_pgf_text_height(vita2d_pgf *font, unsigned int size, const char *text)
{
	int height;
	vita2d_pgf_text_dimensions(font, size, text, NULL, &height);
	return height;
}

void vita2d_pgf_set_linespace(vita2d_pgf *font, int line_space)
{
	font->line_space = line_space;
}

int vita2d_pgf_get_linespace(vita2d_pgf *font)
{
	return font->line_space;
}

int vita2d_pgf_get_lineheight(vita2d_pgf *font, int size)
{
	return size * (float)font->max_height / (float)font->font_size + 0.5f + FONT_GLYPH_MARGIN / 2.0f;
}
