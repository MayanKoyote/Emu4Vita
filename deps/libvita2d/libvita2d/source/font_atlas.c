#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "font_atlas.h"

font_atlas *font_atlas_create(int width, int height, SceGxmTextureFormat format)
{
	font_atlas *atlas = malloc(sizeof(*atlas));
	if (!atlas)
		return NULL;

	texture_entry *tex_entry = malloc(sizeof(*tex_entry));
	if (!tex_entry)
	{
		free(atlas);
		return NULL;
	}

	tex_entry->prev = NULL;
	tex_entry->tex = vita2d_create_empty_texture_format(width, height, format);
	if (!tex_entry->tex)
	{
		free(tex_entry);
		free(atlas);
		return NULL;
	}

	vita2d_texture_set_filters(tex_entry->tex, SCE_GXM_TEXTURE_FILTER_POINT, SCE_GXM_TEXTURE_FILTER_LINEAR);

	atlas->tex_entry = tex_entry;
	atlas->format = format;
	atlas->x = 0;
	atlas->y = 0;
	atlas->width = width;
	atlas->height = height;
	atlas->y_space = 0;

	atlas->htab = int_htab_create(256);

	return atlas;
}

void font_atlas_free(font_atlas *atlas)
{
	texture_entry *tex_entry = atlas->tex_entry;
	while (tex_entry)
	{
		texture_entry *prev = tex_entry->prev;
		if (tex_entry->tex)
			vita2d_free_texture(tex_entry->tex);
		free(tex_entry);
		tex_entry = prev;
	}

	int_htab_free(atlas->htab);
	free(atlas);
}

int font_atlas_insert(font_atlas *atlas, unsigned int character,
					  vita2d_texture **tex, vita2d_position *position,
					  font_glyph *glyph)
{
	atlas_htab_entry *entry = malloc(sizeof(atlas_htab_entry));
	if (!entry)
		return 0;

	if (atlas->x + glyph->bitmap_width > atlas->width)
	{
		atlas->x = 0;
		atlas->y += atlas->y_space;
		atlas->y_space = glyph->bitmap_height;
	}

	if (atlas->y + glyph->bitmap_height > atlas->height)
	{
		texture_entry *tex_entry = malloc(sizeof(*tex_entry));
		if (!tex_entry)
			return 0;

		tex_entry->prev = atlas->tex_entry;
		tex_entry->tex = vita2d_create_empty_texture_format(atlas->width, atlas->height, atlas->format);
		if (!tex_entry->tex)
		{
			free(tex_entry);
			return 0;
		}

		// printf("font_atlas_insert: creat tex_entry, entry = %p, texture = %p\n", tex_entry, tex_entry->tex);

		vita2d_texture_set_filters(tex_entry->tex,
								   SCE_GXM_TEXTURE_FILTER_POINT,
								   SCE_GXM_TEXTURE_FILTER_LINEAR);

		atlas->tex_entry = tex_entry;
		atlas->x = 0;
		atlas->y = 0;
	}

	entry->tex = atlas->tex_entry->tex;
	entry->position.x = atlas->x;
	entry->position.y = atlas->y;
	entry->glyph = *glyph;

	*tex = entry->tex;
	*position = entry->position;

	if (!int_htab_insert(atlas->htab, character, entry))
	{
		free(entry);
		return 0;
	}

	atlas->x += glyph->bitmap_width;
	if (glyph->bitmap_height > atlas->y_space)
		atlas->y_space = glyph->bitmap_height;

	return 1;
}

int font_atlas_exists(font_atlas *atlas, unsigned int character)
{
	return int_htab_find(atlas->htab, character) != NULL;
}

int font_atlas_get(font_atlas *atlas, unsigned int character,
				   vita2d_texture **tex, vita2d_position *position,
				   font_glyph *glyph)
{
	atlas_htab_entry *entry = int_htab_find(atlas->htab, character);
	if (!entry)
		return 0;

	*tex = entry->tex;
	*position = entry->position;
	*glyph = entry->glyph;

	return 1;
}
