#ifndef _OBJECT_H_
#define _OBJECT_H_

#include <cglm/cglm.h>

#include "model.h"
#include "texture.h"

enum {
	OBJECT_FLAG_IS_ACTIVE = (1 << 0),
};

typedef struct {
	vec3 position;
	model_t *mdl;
	uint8_t flags;
} object_t;

object_t object_create_from_model(model_t *mdl, const uint8_t flags);
void object_duplicate(object_t *to, object_t *from);
void object_render(object_t *obj, shader_t *s, const float *proj_matrix,
		   const float *view_matrix, const int use_meshes_aabb,
		   texture_t *textures);
void object_destroy(object_t *obj);

#endif /* _OBJECT_H_ */
