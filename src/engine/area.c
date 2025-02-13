#include "t3d_ext.h"

#include "engine/area.h"

void area_free(struct area *a);

struct area area_init_from_file(FILE *file, T3DModel *scene_model,
				const u16 index)
{
	struct area a;

	for (u8 i = 0; i < 3; i++) {
		fread(a.offset.v + i, 4, 1, file);
	}
	a.colmesh = collision_mesh_init_from_file(file, &a.offset);
	fread(&a.actor_header_count, 2, 1, file);
	a.actor_headers =
		calloc(a.actor_header_count, sizeof(*a.actor_headers));
	for (u16 i = 0; i < a.actor_header_count; i++) {
		a.actor_headers[i] =
			actor_init_from_file(file, &a.offset, index);
	}

	/* respective scene actor */
	char colmesh_name[16];
	memset(colmesh_name, 0, 16);
	snprintf(colmesh_name, 16, "Col.%u", index);
	a.matrix = malloc_uncached(sizeof(*a.matrix));
	t3d_mat4fp_from_srt_euler(a.matrix, (f32[3]){ 1, 1, 1 },
				  (f32[3]){ 0, 0, 0 }, a.offset.v);

	/* scene actor displaylist */
	rspq_block_begin();
	t3d_matrix_push(a.matrix);
	T3DModelIter iter =
		t3d_model_iter_create(scene_model, T3D_CHUNK_TYPE_OBJECT);
	while (t3d_model_iter_next(&iter)) {
		if (strncmp(iter.object->name, colmesh_name, 16)) {
			continue;
		}
		t3d_model_draw_material(iter.object->material, NULL);
		t3d_model_draw_object(iter.object, NULL);
	}
	t3d_matrix_pop(1);
	a.displaylist = rspq_block_end();

#ifdef AREA_DEBUG
	debugf("area_init_from_file(%p, %p, %u)\n", file, scene_model, index);
	debugf("\toffset = (%f, %f, %f)\n", a.offset.v[0], a.offset.v[1],
	       a.offset.v[2]);
	debugf("\tactor_header_count = %d\n", a.actor_header_count);
	debugf("\tactor_headers = %p\n", a.actor_headers);
	debugf("\tdisplaylist = %p\n", a.displaylist);
	debugf("\tmatrix = %p\n", a.matrix);
	debugf("\n");
#endif

	return a;
}

void area_render(const struct area *a, const f32 subtick)
{
	/* actors */
	for (u16 i = 0; i < a->actor_header_count; i++) {
		struct actor_header *ah = a->actor_headers[i];

		if (!(ah->flags & ACTOR_FLAG_IS_ACTIVE)) {
			continue;
		}

		actor_matrix_setup(ah, subtick);
		actor_render(ah);
	}

	/* static geometry */
	rspq_block_run(a->displaylist);
}

void area_free(struct area *a)
{
	free_uncached(a->matrix);
	a->matrix = NULL;
	for (u16 i = 0; i < a->actor_header_count; i++) {
		actor_free(a->actor_headers[i], true);
	}
	free(a->actor_headers);
	a->actor_headers = NULL;
	a->actor_header_count = 0;
	collision_mesh_free(&a->colmesh);
	a->offset = T3D_VEC3_ZERO;
}
