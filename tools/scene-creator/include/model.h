#ifndef _MODEL_H_
#define _MODEL_H_

#include <stdint.h>

#include "mesh.h"
#include "texture.h"

#define MODEL_PATH_MAX_LEN 512

enum {
	MODEL_FLAG_IS_ACTIVE = (1 << 0),
};

typedef struct {
	uint16_t num_meshes;
	mesh_t *meshes;
	vec3 position;
	uint8_t flags;
	char path[MODEL_PATH_MAX_LEN];
} model_t;

model_t model_create_from_file(const char *path, uint8_t flags,
			       uint16_t *num_textures, texture_t *textures);
int models_are_identical(const model_t *a, const model_t *b);
void model_render(model_t *m, const shader_t *s, const float *proj_matrix,
		  const float *view_matrix, vec3 offset,
		  const int use_meshes_aabb, texture_t *textures);
void model_move(model_t *dst, model_t *src);
void model_destroy(model_t *m);

#endif /* _MODEL_H_ */
