#include <GL/glew.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "texture.h"

void texture_load_to_array(const char *path, uint16_t *num_textures,
			   texture_t *textures)
{
	texture_t *tex = textures + (*num_textures)++;
	uint32_t gl_tex;
	tex->pixels = stbi_load(path, &tex->width, &tex->height, &tex->comp, 0);

	/* FIXME: LOOKUP TABLE, MOTHERFUCKERS! */
	switch (tex->comp) {
	case 0:
	case 2:
	default:
		fprintf(stderr,
			"Invalid texture format for '%s' "
			"(%d components)\n",
			path, tex->comp);
		exit(EXIT_FAILURE);
		break;

	case 1:
		tex->format = GL_RED;
		break;

	case 3:
		tex->format = GL_RGB;
		break;

	case 4:
		tex->format = GL_RGBA;
		break;
	}

	/*
	 * POSSIBLE FIXME:
	 * this is a retarded cheat I'm doing in order to get grayscale
	 * textures to show up correctly (otherwise, they're just red).
	 * this also breaks how OpenGL handles internal texture formats,
	 * but that'll only start becoming a problem with RGBA textures.
	 */
	/*
	if (tex->format == GL_RED) {
		tex->comp = 3;
		tex->pixels =
			realloc(tex->pixels, sizeof *tex->pixels * tex->width *
						     tex->height * tex->comp);
		uint8_t *tmpdup = calloc(tex->width * tex->height, 1);
		memcpy(tmpdup, tex->pixels, tex->width * tex->height);
		for (int i = 0; i < tex->width * tex->height; i++) {
			tex->pixels[i * 3 + 0] = tmpdup[i];
			tex->pixels[i * 3 + 1] = tmpdup[i];
			tex->pixels[i * 3 + 2] = tmpdup[i];
		}
		free(tmpdup);
	}
	*/

	if (!tex->pixels) {
		fprintf(stderr, "Failed to load image from '%s' (%s)\n", path,
			stbi_failure_reason());
		exit(EXIT_FAILURE);
	}

	glGenTextures(1, &gl_tex);
	glBindTexture(GL_TEXTURE_2D, gl_tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
			GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	/*
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex->width, tex->height, 0,
		     GL_RGB, GL_UNSIGNED_BYTE, tex->pixels);
	*/
	glTexImage2D(GL_TEXTURE_2D, 0, tex->format, tex->width, tex->height, 0,
		     tex->format, GL_UNSIGNED_BYTE, tex->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	tex->nkimg = nk_image_id(gl_tex);
}

void texture_move(texture_t *to, texture_t *from)
{
	to->width = from->width;
	to->height = from->height;
	to->comp = from->comp;
	to->format = from->format;
	to->nkimg = from->nkimg;

	from->width = from->height = from->comp = from->format = 0;
	memset(&from->nkimg, 0, sizeof from->nkimg);

	if (!to->pixels) {
		to->pixels = calloc(to->width * to->height * to->comp, 1);
	}
	to->pixels = realloc(to->pixels, to->width * to->height * to->comp);
	memcpy(to->pixels, from->pixels,
	       sizeof *to->pixels * to->width * to->height * to->comp);

	if (from->pixels) {
		free(from->pixels);
		from->pixels = NULL;
	}
}

void texture_destroy(texture_t *array, uint16_t index)
{
	texture_t *t = array + index;

	t->width = t->height = t->comp = t->format = t->nkimg.w = t->nkimg.h =
		0;
	memset(t->nkimg.region, 0, sizeof *t->nkimg.region * 4);
	t->nkimg.handle.ptr = NULL;
	glDeleteTextures(1, (const GLuint *)&t->nkimg.handle.id);
	t->nkimg.handle.id = -1;

	if (t->pixels) {
		free(t->pixels);
		t->pixels = NULL;
	}
}
