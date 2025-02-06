#include "engine/area.h"

area_t area_read_from_file(FILE *file)
{
	area_t a;

	a.colmesh = collision_mesh_read_from_file(file);
	fread(&a.num_objects, 2, 1, file);
	a.objects = calloc(a.num_objects, sizeof *a.objects);
	for (uint16_t i = 0; i < a.num_objects; i++) {
		a.objects[i] = object_read_from_file(file);
	}

	return a;
}

void area_render(const area_t *a, const float subtick)
{
	for (uint16_t i = 0; i < a->num_objects; i++) {
		object_t *o = a->objects + i;

		object_matrix_setup(o, subtick);
		object_render(o);
	}
}

void area_terminate(area_t *a)
{
	for (uint16_t i = 0; i < a->num_objects; i++) {
		object_terminate(a->objects + i, true);
	}
	free(a->objects);
	a->objects = NULL;
	a->num_objects = 0;
	collision_mesh_terminate(&a->colmesh);
}
