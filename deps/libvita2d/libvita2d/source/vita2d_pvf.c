#include <psp2/pvf.h>
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

typedef struct vita2d_pvf_font_handle {
	ScePvfFontId font_handle;
	int (*in_font_group)(unsigned int c);
	struct vita2d_pvf_font_handle *next;
} vita2d_pvf_font_handle;

typedef struct vita2d_pvf {
	ScePvfLibId lib_handle;
	vita2d_pvf_font_handle *font_handle_list;
	texture_atlas *atlas;
	SceKernelLwMutexWork mutex;
	int font_size;
	int max_height;
	int max_ascender;
	int max_descender;
	int line_space;
} vita2d_pvf;

static void *pvf_alloc_func(void *userdata, unsigned int size)
{
	return memalign(sizeof(int), (size + sizeof(int) - 1) / sizeof(int) * sizeof(int));
}

static void *pvf_realloc_func(void *userdata, void *old_ptr, unsigned int size)
{
	return realloc(old_ptr, (size + sizeof(int) - 1) / sizeof(int) * sizeof(int));
}

static void pvf_free_func(void *userdata, void *p)
{
	free(p);
}

static int vita2d_load_pvf_post(vita2d_pvf *font)
{
	ScePvfFontInfo fontinfo;

	scePvfGetFontInfo(font->font_handle_list->font_handle, &fontinfo);
	font->font_size = fontinfo.maxFGlyphMetrics.height + 0.5f;
	font->max_height = fontinfo.maxFGlyphMetrics.height + 0.5f;
	font->max_ascender = fontinfo.maxFGlyphMetrics.ascender + 0.5f;
	font->max_descender = fontinfo.maxFGlyphMetrics.descender + 0.5f;
	// printf("[VITA2D_PVF] font->font_size: %d, font->max_height: %d, font->max_ascender = %d, font->max_descender = %d\n",
	//		font->font_size, font->max_height, font->max_ascender, font->max_descender);

	font->atlas = texture_atlas_create(ATLAS_DEFAULT_W, ATLAS_DEFAULT_H,
		SCE_GXM_TEXTURE_FORMAT_U8_R111);

	if (!font->atlas)
		return 0;

	sceKernelCreateLwMutex(&font->mutex, "vita2d_pvf_mutex", 2, 0, NULL);
	return 1;
}

static vita2d_pvf *vita2d_load_pvf_pre(int numFonts)
{
	ScePvfError error;

	vita2d_pvf *font = malloc(sizeof(*font));
	if (!font)
		return NULL;
	memset(font, 0, sizeof(vita2d_pvf));

	ScePvfInitRec params = {
		font,
		numFonts,
		NULL,
		NULL,
		pvf_alloc_func,
		pvf_realloc_func,
		pvf_free_func
	};

	font->lib_handle = scePvfNewLib(&params, &error);
	if (error != 0) {
		free(font);
		return NULL;
	}

	scePvfSetEM(font->lib_handle, 72.0f / (10.125f * 128.0f));
	scePvfSetResolution(font->lib_handle, 128.0f, 128.0f);

	return font;
}

vita2d_pvf *vita2d_load_system_pvf(int numFonts, const vita2d_system_pvf_config *configs)
{
	if (numFonts < 1) {
		return NULL;
	}

	ScePvfError error;
	int i;

	vita2d_pvf *font = vita2d_load_pvf_pre(numFonts);

	if (!font)
		return NULL;

	vita2d_pvf_font_handle *tmp = NULL;

	for (i = 0; i < numFonts; i++) {
		ScePvfFontStyleInfo style;

		memset(&style, 0, sizeof(style));
		style.languageCode = configs[i].code;
		style.familyCode = SCE_PVF_DEFAULT_FAMILY_CODE;
		style.style = SCE_PVF_DEFAULT_STYLE_CODE;

		ScePvfFontIndex index = scePvfFindOptimumFont(font->lib_handle, &style, &error);
		if (error != 0)
			goto cleanup;

		ScePvfFontId font_handle = scePvfOpen(font->lib_handle, index, 0, &error);
		if (error != 0)
			goto cleanup;

		if (font->font_handle_list == NULL) {
			tmp = font->font_handle_list = malloc(sizeof(vita2d_pvf_font_handle));
		} else {
			tmp = tmp->next = malloc(sizeof(vita2d_pvf_font_handle));
		}
		if (!tmp) {
			scePvfClose(font_handle);
			goto cleanup;
		}

		scePvfSetCharSize(font_handle, 10.125f, 10.125f);

		memset(tmp, 0, sizeof(vita2d_pvf_font_handle));
		tmp->font_handle = font_handle;
		tmp->in_font_group = configs[i].in_font_group;
	}

	if (!vita2d_load_pvf_post(font)) {
		vita2d_free_pvf(font);
		return NULL;
	}

	return font;

cleanup:
	tmp = font->font_handle_list;
	while (tmp) {
		vita2d_pvf_font_handle *next = tmp->next;
		scePvfClose(tmp->font_handle);
		free(tmp);
		tmp = next;
	}
	scePvfDoneLib(font->lib_handle);
	free(font);
	return NULL;
}

vita2d_pvf *vita2d_load_default_pvf()
{
	vita2d_system_pvf_config configs[] = {
		{SCE_PVF_DEFAULT_LANGUAGE_CODE, NULL},
	};

	return vita2d_load_system_pvf(1, configs);
}

vita2d_pvf *vita2d_load_custom_pvf(const char *path)
{
	ScePvfError error;
	vita2d_pvf *font = vita2d_load_pvf_pre(1);

	if (!font)
		return NULL;

	vita2d_pvf_font_handle *handle = malloc(sizeof(vita2d_pvf_font_handle));
	if (!handle) {
		free(font);
		return NULL;
	}

	ScePvfFontId font_handle = scePvfOpenUserFile(font->lib_handle, (char *)path, 1, &error);
	if (error != 0) {
		scePvfDoneLib(font->lib_handle);
		free(handle);
		free(font);
		return NULL;
	}

	scePvfSetCharSize(font_handle, 10.125f, 10.125f);

	memset(handle, 0, sizeof(vita2d_pvf_font_handle));
	handle->font_handle = font_handle;
	font->font_handle_list = handle;

	if (!vita2d_load_pvf_post(font)) {
		vita2d_free_pvf(font);
		return NULL;
	}

	return font;
}

void vita2d_free_pvf(vita2d_pvf *font)
{
	if (font) {
		sceKernelDeleteLwMutex(&font->mutex);

		vita2d_pvf_font_handle *tmp = font->font_handle_list;
		while (tmp) {
			vita2d_pvf_font_handle *next = tmp->next;
			scePvfClose(tmp->font_handle);
			free(tmp);
			tmp = next;
		}
		scePvfDoneLib(font->lib_handle);
		if (font->atlas)
			texture_atlas_free(font->atlas);
		free(font);
	}
}

ScePvfFontId get_font_for_character(vita2d_pvf *font, unsigned int character)
{
	ScePvfFontId font_handle = font->font_handle_list->font_handle;
	vita2d_pvf_font_handle *tmp = font->font_handle_list;

	while (tmp) {
		if (tmp->in_font_group == NULL || tmp->in_font_group(character)) {
			font_handle = tmp->font_handle;
			break;
		}
		tmp = tmp->next;
	}

	return font_handle;
}

static int atlas_add_glyph(vita2d_pvf *font, ScePvfFontId font_handle, unsigned int character)
{
	ScePvfCharInfo char_info;
	ScePvfIrect char_image_rect;
	vita2d_position position;
	void *texture_data;
	vita2d_texture *tex = NULL;

	if (scePvfGetCharInfo(font_handle, character, &char_info) < 0)
		return 0;

	if (scePvfGetCharImageRect(font_handle, character, &char_image_rect) < 0)
		return 0;

	vita2d_size size = {
		char_image_rect.width + 2 * FONT_GLYPH_MARGIN,
		char_image_rect.height + 2 * FONT_GLYPH_MARGIN
	};

	texture_atlas_entry_data data = {
		char_info.glyphMetrics.horizontalBearingX64 >> 6,
		char_info.glyphMetrics.horizontalBearingY64 >> 6,
		char_info.glyphMetrics.horizontalAdvance64 >> 6,
		char_info.glyphMetrics.verticalAdvance64 >> 6,
		font->font_size
	};

	if (!texture_atlas_insert(font->atlas, character, &size, &data,
				  &tex, &position))
			return 0;

	texture_data = vita2d_texture_get_datap(tex);

	ScePvfUserImageBufferRec glyph_image;
	glyph_image.pixelFormat = SCE_PVF_USERIMAGE_DIRECT8;
	glyph_image.xPos64 = ((position.x + FONT_GLYPH_MARGIN) << 6) - char_info.glyphMetrics.horizontalBearingX64;
	glyph_image.yPos64 = ((position.y + FONT_GLYPH_MARGIN) << 6) + char_info.glyphMetrics.horizontalBearingY64;
	glyph_image.rect.width = vita2d_texture_get_width(tex);
	glyph_image.rect.height = vita2d_texture_get_height(tex);
	glyph_image.bytesPerLine = vita2d_texture_get_stride(tex);
	glyph_image.reserved = 0;
	glyph_image.buffer = (ScePvfU8 *)texture_data;

	return scePvfGetCharGlyphImage(font_handle, character, &glyph_image) == 0;
}

static int generic_pvf_draw_text(vita2d_pvf *font, int draw, int *height,
									int x, int y, unsigned int color,
									unsigned int size,
									const char *text)
{
	sceKernelLockLwMutex(&font->mutex, 1, NULL);

	int i;
	unsigned int character;
	ScePvfFontId fontid;
	vita2d_rectangle rect;
	texture_atlas_entry_data data;
	ScePvfKerningInfo kerning_info;
	unsigned int old_character = 0;
	vita2d_texture *tex = NULL;
	float scale;
	int start_x = x;
	int max_x = 0;
	int pen_x = x;
	int pen_y = y;
	int line_height = vita2d_pvf_get_lineheight(font, size);

	for (i = 0; text[i];) {
		i += utf8_to_ucs2(&text[i], &character);

		if (character == '\n') {
			if (pen_x > max_x)
				max_x = pen_x;
			pen_x = start_x;
			pen_y += (line_height + font->line_space);
			continue;
		}

		fontid = get_font_for_character(font, character);

		if (!texture_atlas_get(font->atlas, character, &tex, &rect, &data)) {
			if (!atlas_add_glyph(font, fontid, character))
				continue;

			if (!texture_atlas_get(font->atlas, character, &tex, &rect, &data))
					continue;
		}

		if (old_character) {
			if (scePvfGetKerningInfo(fontid, old_character, character, &kerning_info) >= 0) {
				pen_x += kerning_info.fKerningInfo.xOffset;
				pen_y += kerning_info.fKerningInfo.yOffset;
			}
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
		old_character = character;
	}

	if (pen_x > max_x)
		max_x = pen_x;

	if (height)
		*height = pen_y + size - y;

	sceKernelUnlockLwMutex(&font->mutex, 1);

	return max_x - x;
}

int vita2d_pvf_draw_text(vita2d_pvf *font, int x, int y,
			 unsigned int color, unsigned int size,
			 const char *text)
{
	return generic_pvf_draw_text(font, 1, NULL, x, y, color, size, text);
}

int vita2d_pvf_draw_textf(vita2d_pvf *font, int x, int y,
			  unsigned int color, unsigned int size,
			  const char *text, ...)
{
	char buf[1024];
	va_list argptr;
	va_start(argptr, text);
	vsnprintf(buf, sizeof(buf), text, argptr);
	va_end(argptr);
	return vita2d_pvf_draw_text(font, x, y, color, size, buf);
}

void vita2d_pvf_text_dimensions(vita2d_pvf *font, unsigned int size,
				const char *text, int *width, int *height)
{
	int w;
	w = generic_pvf_draw_text(font, 0, height, 0, 0, 0, size, text);

	if (width)
		*width = w;
}

int vita2d_pvf_text_width(vita2d_pvf *font, unsigned int size, const char *text)
{
	int width;
	vita2d_pvf_text_dimensions(font, size, text, &width, NULL);
	return width;
}

int vita2d_pvf_text_height(vita2d_pvf *font, unsigned int size, const char *text)
{
	int height;
	vita2d_pvf_text_dimensions(font, size, text, NULL, &height);
	return height;
}

void vita2d_pvf_set_linespace(vita2d_pvf *font, int line_space)
{
	font->line_space = line_space;
}

int vita2d_pvf_get_linespace(vita2d_pvf *font)
{
	return font->line_space;
}

int vita2d_pvf_get_lineheight(vita2d_pvf *font, int size)
{
	return size * (float)font->max_height / (float)font->font_size + 0.5f + FONT_GLYPH_MARGIN / 2.0f;
}
