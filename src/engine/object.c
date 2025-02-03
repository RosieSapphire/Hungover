#include "t3d_ext.h"

#include "engine/object.h"

object_t object_init_from_model_path(const char *path, const T3DVec3 *pos,
				     const T3DVec3 *rot, const T3DVec3 *scale)
{
	object_t o;

	o.mdl = t3d_model_load(path);
	assertf(o.mdl, "Failed to load model from '%s'\n", path);

	o.matrix = malloc_uncached(sizeof *o.matrix);

	rspq_block_begin();
	t3d_matrix_push(o.matrix);
	t3d_model_draw(o.mdl);
	t3d_matrix_pop(1);
	o.displaylist = rspq_block_end();

	o.position = o.position_old = *pos;
	o.rotation = o.rotation_old = *rot;
	o.scale = o.scale_old = *scale;

	return o;
}

object_t object_init_from_model_pointer(T3DModel *mdl, const T3DVec3 *pos,
					const T3DVec3 *rot,
					const T3DVec3 *scale)
{
	object_t o;

	o.mdl = mdl;

	o.matrix = malloc_uncached(sizeof *o.matrix);

	rspq_block_begin();
	t3d_matrix_push(o.matrix);
	t3d_model_draw(o.mdl);
	t3d_matrix_pop(1);
	o.displaylist = rspq_block_end();

	o.position = o.position_old = *pos;
	o.rotation = o.rotation_old = *rot;
	o.scale = o.scale_old = *scale;

	return o;
}

void object_render(object_t *obj)
{
	rspq_block_run(obj->displaylist);
}

rspq_block_t *objects_instanced_gen_dl(const int num_objs, object_t *objs,
				       const T3DModel *common_mdl)
{
	rspq_block_t *instdl = NULL;

	rspq_block_begin();
	for (int i = 0; i < num_objs; i++) {
		object_t *o = objs + i;
		assertf(o->mdl == common_mdl,
			"Object %d in array of %d does not share "
			"a common model, and cannot be instanced\n",
			i, num_objs);
		object_render(o);
	}
	return (instdl = rspq_block_end());
}

void object_matrix_setup(object_t *o, const float subtick)
{
	T3DVec3 pos_lerp, rot_lerp, scale_lerp;

	t3d_vec3_lerp(&pos_lerp, &o->position_old, &o->position, subtick);
	t3d_vec3_lerp(&rot_lerp, &o->rotation_old, &o->rotation, subtick);
	t3d_vec3_lerp(&scale_lerp, &o->scale_old, &o->scale, subtick);

	t3d_mat4fp_from_srt_euler(o->matrix, scale_lerp.v, rot_lerp.v,
				  pos_lerp.v);
}

void object_terminate(object_t *o, const int should_free_model)
{
	o->position = o->position_old = o->rotation = o->rotation_old =
		o->scale = o->scale_old = T3D_VEC3_ZERO;

	free_uncached(o->matrix);
	o->matrix = NULL;

	rspq_block_free(o->displaylist);
	o->displaylist = NULL;

	if (should_free_model) {
		t3d_model_free(o->mdl);
	}
	o->mdl = NULL;
}
