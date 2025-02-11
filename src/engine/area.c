#include "t3d_ext.h"

#include "engine/area.h"

Area areaInitFromFile(FILE *file, T3DModel *sceneModel, const int index)
{
	Area a;

	for (int i = 0; i < 3; i++) {
		fread(a.offset.v + i, 4, 1, file);
	}
	a.colmesh = collisionMeshInitFromFile(file, &a.offset);
	fread(&a.numObjects, 2, 1, file);
	a.objects = calloc(a.numObjects, sizeof *a.objects);
	for (uint16_t i = 0; i < a.numObjects; i++) {
		a.objects[i] = objectInitFromFile(file, &a.offset, index);
	}

	/* respective scene object */
	char colName[16];
	memset(colName, 0, 16);
	snprintf(colName, 16, "Col.%u", index);
	a.matrix = malloc_uncached(sizeof *a.matrix);
	t3d_mat4fp_from_srt_euler(a.matrix, (float[3]){ 1, 1, 1 },
				  (float[3]){ 0, 0, 0 }, a.offset.v);

	/* scene obj displaylist */
	rspq_block_begin();
	t3d_matrix_push(a.matrix);
	T3DModelIter iter =
		t3d_model_iter_create(sceneModel, T3D_CHUNK_TYPE_OBJECT);
	while (t3d_model_iter_next(&iter)) {
		if (strncmp(iter.object->name, colName, 16)) {
			continue;
		}
		t3d_model_draw_material(iter.object->material, NULL);
		t3d_model_draw_object(iter.object, NULL);
	}
	t3d_matrix_pop(1);
	a.areaBlock = rspq_block_end();

	return a;
}

void areaRender(const Area *a, const float subtick)
{
	/* objects */
	for (uint16_t i = 0; i < a->numObjects; i++) {
		Object *o = a->objects + i;

		if (!(o->flags & OBJECT_FLAG_IS_ACTIVE)) {
			continue;
		}

		objectMatrixSetup(o, subtick);
		objectRender(o);
	}

	/* static geometry */
	rspq_block_run(a->areaBlock);
}

void areaFree(Area *a)
{
	free_uncached(a->matrix);
	a->matrix = NULL;
	for (uint16_t i = 0; i < a->numObjects; i++) {
		objectFree(a->objects + i, true);
	}
	free(a->objects);
	a->objects = NULL;
	a->numObjects = 0;
	collisionMeshFree(&a->colmesh);
	a->offset = T3D_VEC3_ZERO;
}
