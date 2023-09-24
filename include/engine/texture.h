#ifndef ENGINE_TEXTURE_H_
#define ENGINE_TEXTURE_H_

#include <GL/gl.h>
#include <libdragon.h>

typedef struct {
	GLuint id;
	sprite_t *spr;
	surface_t surf;
} texture_t;

void textures_init(void);
texture_t texture_create_empty(int fmt, int width, int height);
texture_t texture_create_file(const char *path);
void texture_destroy(texture_t *t);

#endif /* ENGINE_TEXTURE_H_ */
