#include "engine/scene.h"

scene_t scene_init_from_file(const char *path)
{
	scene_t scn;

	char mdl_name[32];
	char mdl_path[64];
	memset(mdl_name, 0, 32);
	memset(mdl_path, 0, 64);

	strncpy(mdl_name, path, strlen(path) - 4);
	snprintf(mdl_path, 64, "%s.t3dm", mdl_name);
	scn.mdl = t3d_model_load(mdl_path);

	scn.area_index = scn.area_index_old = 0;
	scn.flags = 0;

	FILE *file = asset_fopen(path, NULL);
	assertf(file, "Failed to load scene from '%s'\n", path);

	fread(&scn.num_areas, 2, 1, file);
	// debugf("Scene '%s' has %d areas\n", path, scn.num_areas);
	scn.areas = calloc(scn.num_areas, sizeof *scn.areas);
	for (uint16_t i = 0; i < scn.num_areas; i++) {
		scn.areas[i] = area_read_from_file(file, scn.mdl, i);
	}

	fclose(file);

	return scn;
}

void scene_update(scene_t *scn, const T3DVec3 *player_pos, const float dt)
{
	uint16_t *inds[2] = { &scn->area_index, &scn->area_index_old };

	for (uint16_t i = 0; i < 2; i++) {
		if (i && !(scn->flags & SCENE_FLAG_PROCESS_AREA_LAST)) {
			continue;
		}

		area_t *area = scn->areas + *inds[i];
		for (uint16_t j = 0; j < area->num_objects; j++) {
			object_t *o = area->objects + j;

			if (!(o->flags & OBJECT_FLAG_IS_ACTIVE)) {
				continue;
			}

			switch (object_update(o, player_pos, dt)) {
			case OBJECT_UPDATE_RETURN_LOAD_NEXT_AREA:
				debugf("LOAD NEXT AREA (%d)\n",
				       o->argi[OBJECT_DOOR_ARGI_NEXT_AREA]);
				*inds[1] = *inds[0];
				*inds[0] = o->argi[OBJECT_DOOR_ARGI_NEXT_AREA];
				scn->flags |= SCENE_FLAG_PROCESS_AREA_LAST;
				object_t *new_door = area_find_door_by_dest_index(
					scn->areas +
						o->argi[OBJECT_DOOR_ARGI_NEXT_AREA],
					*inds[1]);
				new_door->flags &= ~(OBJECT_FLAG_IS_ACTIVE);
				debugf("Deactivated next door\n");
				break;

			case OBJECT_UPDATE_RETURN_UNLOAD_PREV_AREA:
				debugf("UNLOAD PREV AREA (%d)\n", *inds[1]);
				scn->flags &= ~(SCENE_FLAG_PROCESS_AREA_LAST);
				object_t *cur_door = area_find_door_by_dest_index(
					scn->areas +
						o->argi[OBJECT_DOOR_ARGI_NEXT_AREA],
					*inds[1]);
				cur_door->flags |=
					OBJECT_FLAG_IS_ACTIVE |
					OBJECT_FLAG_MUST_REENTER_RADIUS_TO_INTERACT;
				break;

			default:
				break;
			}
		}
	}
}

void scene_render(const scene_t *scn, const float subtick)
{
	area_render(scn->areas + scn->area_index, subtick);
	if ((scn->flags & SCENE_FLAG_PROCESS_AREA_LAST) &&
	    scn->area_index ^ scn->area_index_old) {
		area_render(scn->areas + scn->area_index_old, subtick);
	}
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
