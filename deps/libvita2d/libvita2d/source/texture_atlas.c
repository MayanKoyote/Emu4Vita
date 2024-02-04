#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "texture_atlas.h"

texture_atlas *texture_atlas_create(int width, int height, SceGxmTextureFormat format)
{
	texture_atlas *atlas = malloc(sizeof(*atlas));
	if (!atlas)
		return NULL;

	atlas_texture_entry *tex_entry = malloc(sizeof(*tex_entry));
	if (!tex_entry) {
		free(atlas);
		return NULL;
	}

	tex_entry->prev = NULL;
	tex_entry->tex = vita2d_create_empty_texture_format(width,
							    height,
							    format);
	if (!tex_entry->tex) {
		free(tex_entry);
		free(atlas);
		return NULL;
	}

	vita2d_texture_set_filters(tex_entry->tex,
				   SCE_GXM_TEXTURE_FILTER_POINT,
				   SCE_GXM_TEXTURE_FILTER_LINEAR);

	atlas->texture_data.tex_entry = tex_entry;
	atlas->texture_data.format= format;
	atlas->texture_data.rect.x = 0;
	atlas->texture_data.rect.y = 0;
	atlas->texture_data.rect.w = width;
	atlas->texture_data.rect.h = height;
	atlas->texture_data.y_space = 0;

	atlas->htab = int_htab_create(256);

	return atlas;
}

void texture_atlas_free(texture_atlas *atlas)
{
	atlas_texture_entry *tex_entry = atlas->texture_data.tex_entry;
	while (tex_entry){
		atlas_texture_entry *prev = tex_entry->prev;
		// printf("[VITA2D] Free tex_entry: entry = %p, tex = %p\n", tex_entry, tex_entry->tex);
		if (tex_entry->tex)
			vita2d_free_texture(tex_entry->tex);
		free(tex_entry);
		tex_entry = prev;
	}

	int_htab_free(atlas->htab);
	free(atlas);
}

int texture_atlas_insert(texture_atlas *atlas, unsigned int character,
			 const vita2d_size *size,
			 const texture_atlas_entry_data *data,
			 vita2d_texture **tex,
			 vita2d_position *inserted_pos)
{
	atlas_htab_entry *entry = malloc(sizeof(*entry));

	if (atlas->texture_data.rect.x + size->w > atlas->texture_data.rect.w)
	{
		atlas->texture_data.rect.x = 0;
		atlas->texture_data.rect.y += atlas->texture_data.y_space;
	}

	if (atlas->texture_data.rect.y + size->h > atlas->texture_data.rect.h)
	{
		atlas_texture_entry *tex_entry = malloc(sizeof(*tex_entry));
		if (!tex_entry) {
			return 0;
		}

		tex_entry->prev = atlas->texture_data.tex_entry;
		tex_entry->tex = vita2d_create_empty_texture_format(atlas->texture_data.rect.w,
									atlas->texture_data.rect.h,
									atlas->texture_data.format);
								
		if (!tex_entry->tex) {
			free(tex_entry);
			return 0;
		}

		// printf("[VITA2D] Creat tex_entry: entry = %p, tex = %p\n", tex_entry, tex_entry->tex);

		vita2d_texture_set_filters(tex_entry->tex,
				   SCE_GXM_TEXTURE_FILTER_POINT,
				   SCE_GXM_TEXTURE_FILTER_LINEAR);

		atlas->texture_data.tex_entry = tex_entry;
		atlas->texture_data.rect.x = 0;
		atlas->texture_data.rect.y = 0;

	}

	*tex = atlas->texture_data.tex_entry->tex;
	inserted_pos->x = atlas->texture_data.rect.x;
	inserted_pos->y = atlas->texture_data.rect.y;

	entry->tex = *tex;
	entry->rect.x = inserted_pos->x;
	entry->rect.y = inserted_pos->y;
	entry->rect.w = size->w;
	entry->rect.h = size->h;
	entry->data = *data;

	if (!int_htab_insert(atlas->htab, character, entry)) {
		return 0;
	}

	atlas->texture_data.rect.x += size->w;
	if (size->h > atlas->texture_data.y_space)
		atlas->texture_data.y_space = size->h;

	return 1;
}

int texture_atlas_exists(texture_atlas *atlas, unsigned int character)
{
	return int_htab_find(atlas->htab, character) != NULL;
}

int texture_atlas_get(texture_atlas *atlas, unsigned int character, vita2d_texture **tex, 
		      vita2d_rectangle *rect, texture_atlas_entry_data *data)
{
	atlas_htab_entry *entry = int_htab_find(atlas->htab, character);
	if (!entry)
		return 0;

	*tex = entry->tex;
	*rect = entry->rect;
	*data = entry->data;

	return 1;
}
