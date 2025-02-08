#include "t3d_ext.h"

#include "engine/area.h"

area_t area_read_from_file(FILE *file, T3DModel *scene_mdl, const int index)
{
	area_t a;

	for (int i = 0; i < 3; i++) {
		fread(a.offset.v + i, 4, 1, file);
	}
	a.colmesh = collision_mesh_read_from_file(file, &a.offset);
	fread(&a.num_objects, 2, 1, file);
	a.objects = calloc(a.num_objects, sizeof *a.objects);
	for (uint16_t i = 0; i < a.num_objects; i++) {
		a.objects[i] = object_read_from_file(file, &a.offset);
	}

	/* respective scene object */
	char col_name[16];
	memset(col_name, 0, 16);
	snprintf(col_name, 16, "Col.%u", index);
	a.scene_material_ptr = t3d_model_get_material(scene_mdl, "mat");
	a.scene_obj_ptr = t3d_model_get_object(scene_mdl, col_name);
	a.matrix = malloc_uncached(sizeof *a.matrix);
	t3d_mat4fp_from_srt_euler(a.matrix, (float[3]){ 1, 1, 1 },
				  (float[3]){ 0, 0, 0 }, a.offset.v);

	/* scene obj displaylist */
	rspq_block_begin();
	t3d_matrix_push(a.matrix);
	t3d_model_draw_material(a.scene_material_ptr, NULL);
	t3d_model_draw_object(a.scene_obj_ptr, NULL);
	t3d_matrix_pop(1);
	a.scene_obj_ptr->userBlock = rspq_block_end();

	return a;
}

object_t *area_find_door_by_dest_index(area_t *a, const uint16_t dest_index)
{
	for (uint16_t i = 0; i < a->num_objects; i++) {
		object_t *obj = a->objects + i;

		if (obj->type != OBJECT_TYPE_DOOR) {
			continue;
		}

		if (obj->argi[OBJECT_DOOR_ARGI_NEXT_AREA] != dest_index) {
			continue;
		}

		return obj;
	}

	return NULL;
}

void area_render(const area_t *a, const float subtick)
{
	/* objects */
	for (uint16_t i = 0; i < a->num_objects; i++) {
		object_t *o = a->objects + i;

		if (!(o->flags & OBJECT_FLAG_IS_ACTIVE)) {
			continue;
		}

		object_matrix_setup(o, subtick);
		object_render(o);
	}

	/* static geometry */
	rspq_block_run(a->scene_obj_ptr->userBlock);
}

void area_terminate(area_t *a)
{
	free_uncached(a->matrix);
	a->matrix = NULL;
	for (uint16_t i = 0; i < a->num_objects; i++) {
		object_terminate(a->objects + i, true);
	}
	free(a->objects);
	a->objects = NULL;
	a->num_objects = 0;
	collision_mesh_terminate(&a->colmesh);
	a->offset = T3D_VEC3_ZERO;
}
