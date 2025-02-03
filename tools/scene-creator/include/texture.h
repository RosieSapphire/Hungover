#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include <stdint.h>

#undef NK_IMPLEMENTATION
#include "nuklear.h"

typedef struct {
	int width, height, comp;
	uint8_t *pixels;
	int format;
	struct nk_image nkimg;
} texture_t;

void texture_load_to_array(const char *path, uint16_t *num_textures,
			   texture_t *textures);
void texture_move(texture_t *to, texture_t *from);
void texture_destroy(texture_t *array, uint16_t index);

#endif /* _TEXTURE_H_ */
