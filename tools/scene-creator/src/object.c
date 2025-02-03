#include "object.h"

object_t object_create_from_model(model_t *mdl, const uint8_t flags)
{
	object_t obj;

	glm_vec3_zero(obj.position);
	obj.mdl = mdl;
	obj.flags = flags;

	return obj;
}

void object_duplicate(object_t *to, object_t *from)
{
	to->mdl = from->mdl;
	to->flags = from->flags;
	glm_vec3_copy(from->position, to->position);
}

void object_render(object_t *obj, shader_t *s, const float *proj_matrix,
		   const float *view_matrix, const int use_meshes_aabb,
		   texture_t *textures)
{
	if (!(obj->flags & OBJECT_FLAG_IS_ACTIVE)) {
		return;
	}

	model_render(obj->mdl, s, proj_matrix, view_matrix, obj->position,
		     use_meshes_aabb, textures);
}

void object_destroy(object_t *obj)
{
	glm_vec3_zero(obj->position);

	/* We're not freeing the mdl pointer because
	 * other objects could be using it */
	obj->mdl = NULL;

	obj->flags = 0;
}
