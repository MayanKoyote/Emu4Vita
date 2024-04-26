#ifndef FONT_ATLAS_H
#define FONT_ATLAS_H

#include "vita2d.h"
#include "int_htab.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct texture_entry
{
	struct texture_entry *prev;
	vita2d_texture *tex;
} texture_entry;

typedef struct font_glyph
{
	int bitmap_width;
	int bitmap_height;
	int bitmap_left;
	int bitmap_top;
	int advance_x;
	int advance_y;
	int font_size;
} font_glyph;

typedef struct atlas_htab_entry
{
	vita2d_texture *tex;
	vita2d_position position;
	font_glyph glyph;
} atlas_htab_entry;

typedef struct font_atlas
{
	texture_entry *tex_entry;
	SceGxmTextureFormat format;
	int width;
	int height;
	int x;
	int y;
	int y_space;
	int_htab *htab;
} font_atlas;

font_atlas *font_atlas_create(int width, int height, SceGxmTextureFormat format);
void font_atlas_free(font_atlas *atlas);
int font_atlas_insert(font_atlas *atlas, unsigned int character, 
					  vita2d_texture **tex, vita2d_position *position,
					  font_glyph *glyph);
int font_atlas_exists(font_atlas *atlas, unsigned int character);
int font_atlas_get(font_atlas *atlas, unsigned int character,
				   vita2d_texture **tex, vita2d_position *position,
				   font_glyph *glyph);

#ifdef __cplusplus
}
#endif

#endif
