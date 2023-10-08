#ifndef ENGINE_TEXTURE_H_
#define ENGINE_TEXTURE_H_

#include <GL/gl.h>
#include <libdragon.h>

#define TEX_PATH_MAX_LEN 64
#define TEX_PATH_MAX_CNT 8

typedef struct {
	GLuint id;
	sprite_t *spr;
	surface_t surf;
} texture_t;

extern unsigned int num_texs_loaded;
extern char tex_paths_loaded[TEX_PATH_MAX_CNT][TEX_PATH_MAX_LEN];
extern texture_t *tex_objs_loaded;

void textures_init(void);
texture_t texture_create_empty(int fmt, int width, int height);
uint32_t texture_create_file(const char *path);
void texture_destroy(texture_t *t);

#endif /* ENGINE_TEXTURE_H_ */
