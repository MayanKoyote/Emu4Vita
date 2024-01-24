#ifndef TEXTURE_ATLAS_H
#define TEXTURE_ATLAS_H

#include "vita2d.h"
#include "int_htab.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct atlas_position {
	int x, y;
} atlas_position;

typedef struct atlas_size {
	int w, h;
} atlas_size;

typedef struct atlas_rectangle {
	int x, y, w, h;
} atlas_rectangle;

typedef struct atlas_texture_entry {
	struct atlas_texture_entry *prev;
	vita2d_texture *tex;
} atlas_texture_entry;

typedef struct atlas_texture_data {
	atlas_texture_entry *tex_entry;
	SceGxmTextureFormat format;
	atlas_rectangle rect;
	int y_space;
} atlas_texture_data;

typedef struct texture_atlas_entry_data {
	int bitmap_left;
	int bitmap_top;
	int advance_x;
	int advance_y;
	int glyph_size;
} texture_atlas_entry_data;

typedef struct texture_atlas_htab_entry {
	vita2d_texture *tex;
	atlas_rectangle rect;
	texture_atlas_entry_data data;
} atlas_htab_entry;

typedef struct texture_atlas {
	atlas_texture_data texture_data;
	int_htab *htab;
} texture_atlas;

texture_atlas *texture_atlas_create(int width, int height, SceGxmTextureFormat format);
void texture_atlas_free(texture_atlas *atlas);
int texture_atlas_insert(texture_atlas *atlas, unsigned int character,
			 const atlas_size *size,
			 const texture_atlas_entry_data *data,
			 vita2d_texture **tex,
			 atlas_position *inserted_pos);

int texture_atlas_exists(texture_atlas *atlas, unsigned int character);
int texture_atlas_get(texture_atlas *atlas, unsigned int character,
		      vita2d_texture **tex, atlas_rectangle *rect, texture_atlas_entry_data *data);


#ifdef __cplusplus
}
#endif

#endif
