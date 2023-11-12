#include <string.h>
#include <malloc.h>
#include <GL/gl.h>

#include "engine/types.h"
#include "engine/texture.h"

u16 num_texs_loaded;
char tex_paths_loaded[TEX_PATH_MAX_CNT][TEX_PATH_MAX_LEN];
struct texture *tex_objs_loaded;

/**
 * textures_init - Initializes Texture Subsystem
 */
void textures_init(void)
{
	memset(tex_paths_loaded, 0, TEX_PATH_MAX_CNT * TEX_PATH_MAX_LEN);
	tex_objs_loaded = malloc(0);
}

/**
 * texture_create_empty - Creates an Empty Texture
 * @fmt: Image Format
 * @width: Image Width
 * @height: Image Height
 *
 * Return: The Empty Texture
 */
struct texture texture_create_empty(int fmt, int width, int height)
{
	struct texture t;

	t.surf = surface_alloc(fmt, width, height);
	glGenTextures(1, &t.id);
	glBindTexture(GL_TEXTURE_2D, t.id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSurfaceTexImageN64(GL_TEXTURE_2D, 0, &t.surf, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	return (t);
}

/**
 * texture_create_file - Loads a Texture from a File
 * @path: Texture Path
 *
 * Return: OpenGL ID for Texture at Index
 */
u32 texture_create_file(const char *path)
{
	struct texture t;

	for (u16 i = 0; i < num_texs_loaded; i++)
	{
		if (!strcmp(path, tex_paths_loaded[i]))
			return (i);
	}

	num_texs_loaded++;
	tex_objs_loaded = realloc(tex_objs_loaded,
			sizeof(struct texture) * num_texs_loaded);
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

	strcpy(tex_paths_loaded[num_texs_loaded - 1], path);
	tex_objs_loaded[num_texs_loaded - 1] = t;
	return (num_texs_loaded - 1);
}

/**
 * texture_destroy - Destroys a Texture
 * @t: Texture in Question
 */
void texture_destroy(struct texture *t)
{
	glDeleteTextures(1, &t->id);
	sprite_free(t->spr);
	num_texs_loaded--;
	tex_objs_loaded = realloc(tex_objs_loaded,
			sizeof(struct texture) * num_texs_loaded);
}
