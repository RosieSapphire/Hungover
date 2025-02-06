#include "t3d_ext.h"

#include "engine/object.h"

#define OBJECT_NAME_MAX_LENGTH 32

object_t object_read_from_file(FILE *file)
{
	object_t o;
	char obj_name[OBJECT_NAME_MAX_LENGTH];
	char mdl_path[OBJECT_NAME_MAX_LENGTH << 1];

	fread(obj_name, 1, OBJECT_NAME_MAX_LENGTH, file);
	snprintf(mdl_path, OBJECT_NAME_MAX_LENGTH << 1, "rom:/%s.t3dm",
		 obj_name);

	o.mdl = t3d_model_load(mdl_path);
	o.matrix = malloc_uncached(sizeof *o.matrix);

	rspq_block_begin();
	t3d_matrix_push(o.matrix);
	t3d_model_draw(o.mdl);
	t3d_matrix_pop(1);
	o.displaylist = rspq_block_end();

	T3DVec3 pos, rot, scale;

	for (int i = 0; i < 3; i++) {
		fread(pos.v + i, 4, 1, file);
	}

	for (int i = 0; i < 3; i++) {
		fread(rot.v + i, 4, 1, file);
	}

	for (int i = 0; i < 3; i++) {
		fread(scale.v + i, 4, 1, file);
	}

	o.position = o.position_old = pos;
	o.rotation = o.rotation_old = rot;
	o.scale = o.scale_old = scale;

	debugf("\t\tObject '%s':\n", obj_name);
	debugf("\t\t\tPosition: (%f, %f, %f):\n", o.position.v[0],
	       o.position.v[1], o.position.v[2]);
	debugf("\t\t\tRotation: (%f, %f, %f):\n", o.rotation.v[0],
	       o.rotation.v[1], o.rotation.v[2]);
	debugf("\t\t\tScale: (%f, %f, %f):\n", o.scale.v[0], o.scale.v[1],
	       o.scale.v[2]);

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
