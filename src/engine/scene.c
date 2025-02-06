#include "engine/scene.h"

scene_t scene_init_from_file(const char *path)
{
	scene_t scn;
	FILE *file = asset_fopen(path, NULL);
	assertf(file, "Failed to load scene from '%s'\n", path);

	fread(&scn.num_areas, 2, 1, file);
	debugf("Scene '%s' has %d areas\n", path, scn.num_areas);
	scn.areas = calloc(scn.num_areas, sizeof *scn.areas);
	for (uint16_t i = 0; i < scn.num_areas; i++) {
		scn.areas[i] = area_read_from_file(file);
	}

	char mdl_name[32];
	char mdl_path[64];
	memset(mdl_name, 0, 32);
	memset(mdl_path, 0, 64);

	strncpy(mdl_name, path, strlen(path) - 4);
	snprintf(mdl_path, 64, "%s.t3dm", mdl_name);
	scn.mdl = t3d_model_load(mdl_path);

	scn.area_index = 0;

	return scn;
}

void scene_render(const scene_t *scn, const float subtick)
{
	/* this renders only the area's objects */
	area_render(scn->areas + scn->area_index, subtick);

	/* this renders the static collision geometry */
	char col_name[16];
	memset(col_name, 0, 16);
	snprintf(col_name, 16, "Col.%u", scn->area_index);
	t3d_model_draw_object(t3d_model_get_object(scn->mdl, col_name), NULL);
}

void scene_terminate(scene_t *scn)
{
	for (uint16_t i = 0; i < scn->num_areas; i++) {
		area_terminate(scn->areas + i);
	}
	free(scn->areas);
	scn->areas = NULL;
	scn->num_areas = 0;
}
