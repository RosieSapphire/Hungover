#ifndef _ENGINE_TEXTURE_H_
#define _ENGINE_TEXTURE_H_

#include <libdragon.h>

#include "engine/types.h"

#define TEX_PATH_MAX_LEN 64
#define TEX_PATH_MAX_CNT 8

/**
 * struct texture - Texture Struct
 * @id: OpenGL Texture ID
 * @spr: Libdragon Sprite
 * @surf: Libdragon Surface
 */
struct texture
{
	u32 id;
	sprite_t *spr;
	surface_t surf;
};

extern u16 num_texs_loaded;
extern char tex_paths_loaded[TEX_PATH_MAX_CNT][TEX_PATH_MAX_LEN];
extern struct texture *tex_objs_loaded;

void textures_init(void);
struct texture texture_create_empty(int fmt, int width, int height);
u32 texture_create_file(const char *path);
void texture_destroy(struct texture *t);

#endif /* _ENGINE_TEXTURE_H_ */
