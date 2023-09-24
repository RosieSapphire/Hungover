#include <string.h>
#include <malloc.h>

#include "engine/texture.h"

static unsigned int num_texs_loaded = 0;
static const char **tex_paths_loaded = NULL;
static texture_t *tex_objs_loaded = NULL;

void textures_init(void)
{
	tex_paths_loaded = malloc(0);
	tex_objs_loaded = malloc(0);
}

texture_t texture_create_empty(int fmt, int width, int height)
{
	texture_t t;
	t.surf = surface_alloc(fmt, width, height);

	glGenTextures(1, &t.id);
	glBindTexture(GL_TEXTURE_2D, t.id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSurfaceTexImageN64(GL_TEXTURE_2D, 0, &t.surf, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	return t;
}

texture_t texture_create_file(const char *path)
{
	texture_t t;

	for(unsigned int i = 0; i < num_texs_loaded; i++) {
		if(strcmp(tex_paths_loaded[i], path))
			continue;

		return tex_objs_loaded[i];
	}

	num_texs_loaded++;
	tex_paths_loaded = realloc(tex_paths_loaded,
			sizeof(const char *) * num_texs_loaded);
	tex_objs_loaded = realloc(tex_objs_loaded,
			sizeof(texture_t) * num_texs_loaded);
	t.spr = sprite_load(path);

	glGenTextures(1, &t.id);
	glBindTexture(GL_TEXTURE_2D, t.id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	rdpq_texparms_t parms = {
		.s.repeats = REPEAT_INFINITE,
		.t.repeats = REPEAT_INFINITE
	};
	glSpriteTextureN64(GL_TEXTURE_2D, t.spr, &parms);
	glBindTexture(GL_TEXTURE_2D, 0);

	tex_objs_loaded[num_texs_loaded - 1] = t;
	tex_paths_loaded[num_texs_loaded - 1] = path;

	return t;
}

void texture_destroy(texture_t *t)
{
	glDeleteTextures(1, &t->id);
	sprite_free(t->spr);
	num_texs_loaded--;
	tex_paths_loaded = realloc(tex_paths_loaded,
			sizeof(const char *) * num_texs_loaded);
	tex_objs_loaded = realloc(tex_objs_loaded,
			sizeof(texture_t) * num_texs_loaded);
}
