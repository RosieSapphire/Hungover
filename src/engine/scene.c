#include "engine/scene.h"
#include "engine/actor_door.h"

#define SCENE_DEBUG

struct scene scene_init_from_file(const char *path)
{
	struct scene scn;
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

	fread(&scn.area_count, 2, 1, file);
	scn.areas = calloc(scn.area_count, sizeof(*scn.areas));
	for (u16 i = 0; i < scn.area_count; i++) {
		scn.areas[i] = area_init_from_file(file, scn.mdl, i);
	}

	fclose(file);

#ifdef SCENE_DEBUG
	debugf("scene_init_from_file('%s'):\n", path);
	debugf("\tarea_count = %d\n", scn.area_count);
	debugf("\tareas = %p\n", scn.areas);
	debugf("\tmdl = %p\n", scn.mdl);
	debugf("\tarea_index = %d\n", scn.area_index);
	debugf("\tarea_index_old = %d\n", scn.area_index_old);
	debugf("\tflags = %d\n", scn.flags);
#endif

	return scn;
}

static void _scene_area_actor_update(struct actor_header *actor,
				     struct scene *scn,
				     const T3DVec3 *player_pos,
				     const T3DVec3 *player_dir, const float dt)
{
	if (!(actor->flags & ACTOR_FLAG_IS_ACTIVE) ||
	    (actor->flags & ACTOR_FLAG_WAS_UPDATED_THIS_FRAME)) {
		return;
	}

	struct area *area_proc = NULL;
	struct actor_door *door = NULL;

	switch (actor_update(actor, player_pos, player_dir, dt)) {
	case ACTOR_RETURN_LOAD_NEXT_AREA:
		door = actor_doors + actor->type_index;
		scn->area_index_old = scn->area_index;
		scn->area_index = door->area_next;
		scn->flags |= SCENE_FLAG_PROCESS_AREA_LAST;
		struct actor_door *door_new =
			actor_door_find_by_area_next(scn->area_index_old);
		area_proc = scn->areas + scn->area_index;
		for (u16 i = 0; i < area_proc->actor_header_count; i++) {
			struct actor_header *actor =
				area_proc->actor_headers[i];
			if ((actor_doors + actor->type_index) == door_new) {
				actor->flags &= ~(ACTOR_FLAG_IS_ACTIVE);
			}
		}
		return;

	case ACTOR_RETURN_UNLOAD_PREV_AREA:
		door = actor_doors + actor->type_index;
		scn->flags &= ~(SCENE_FLAG_PROCESS_AREA_LAST);
		struct actor_door *door_cur =
			actor_door_find_by_area_next(scn->area_index_old);
		area_proc = scn->areas + door->area_next;
		for (u16 i = 0; i < area_proc->actor_header_count; i++) {
			struct actor_header *actor =
				area_proc->actor_headers[i];
			if ((actor_doors + actor->type_index) == door_cur) {
				actor->flags |= ACTOR_FLAG_IS_ACTIVE;
			}
		}
		return;

	case ACTOR_RETURN_UNLOAD_NEXT_AREA:
		scn->flags &= ~(SCENE_FLAG_PROCESS_AREA_LAST);
		scn->area_index = scn->area_index_old;
		return;

	default:
		return;
	}
}

void scene_update(struct scene *scn, const T3DVec3 *player_pos,
		  const T3DVec3 *player_dir, const f32 dt)
{
	/*
  	 * This is to ensure all actors are only updated once per frame.
  	 * The reason this is done is because if, say, a door were to
  	 * load a new area, the previous area would be active as well,
  	 * causing that same door actor to be updated twice,
  	 * causing various issues. This loop mitigates that.
  	 */
	actor_static_vars_setup();
	for (u16 i = 0; i < scn->area_count; i++) {
		struct area *a = scn->areas + i;

		for (u16 j = 0; j < a->actor_header_count; j++) {
			struct actor_header *actor = a->actor_headers[j];

			actor->flags &= ~(ACTOR_FLAG_WAS_UPDATED_THIS_FRAME);
		}
	}

	for (u16 i = 0; i < 2; i++) {
		if (i && !(scn->flags & SCENE_FLAG_PROCESS_AREA_LAST)) {
			continue;
		}

		struct area *area = scn->areas + (!i ? scn->area_index :
						       scn->area_index_old);

		for (u16 j = 0; j < area->actor_header_count; j++) {
			_scene_area_actor_update(area->actor_headers[j], scn,
						 player_pos, player_dir, dt);
		}
	}
	actor_static_vars_to_ui();
}

void scene_render(const struct scene *scn, const f32 subtick)
{
	area_render(scn->areas + scn->area_index, subtick);
	if ((scn->flags & SCENE_FLAG_PROCESS_AREA_LAST) &&
	    scn->area_index ^ scn->area_index_old) {
		area_render(scn->areas + scn->area_index_old, subtick);
	}
}

void scene_free(struct scene *scn)
{
	for (u16 i = 0; i < scn->area_count; i++) {
		area_free(scn->areas + i);
	}
	free(scn->areas);
	scn->areas = NULL;
	scn->area_count = 0;
}
