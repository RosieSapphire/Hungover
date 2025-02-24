#include "engine/actor_static.h"
#include "engine/actor_door.h"
#include "engine/actor_microwave.h"
#include "engine/actor_pickup.h"

#include "engine/player.h"
#include "engine/scene.h"

// #define SCENE_DEBUG

struct scene scene_init_from_file(const char *path)
{
	struct scene scn;
	const char *ext_str = strrchr(path, '.');
	assertf(ext_str, "Scene path '%s' has no extension!\n", path);

	char mdl_path[64];
	memset(mdl_path, 0, 64);

	const u8 mdl_name_size = (ext_str - path) + 1;
	snprintf(mdl_path, mdl_name_size, path);
	snprintf(mdl_path + mdl_name_size - 1, 6, ".t3dm");

	scn.mdl = t3d_model_load(mdl_path);
	scn.area_index = 0;
	scn.area_index_old = 0;
	scn.flags = 0;

	FILE *file = asset_fopen(path, NULL);
	assertf(file, "Failed to open scene file from '%s'\n", path);

	fread(&scn.area_count, 2, 1, file);
	scn.areas = calloc(scn.area_count, sizeof(*scn.areas));
	for (u16 i = 0; i < scn.area_count; i++) {
		struct area *area = scn.areas + i;
		for (u16 j = 0; j < 3; j++) {
			fread(area->offset.v + j, 4, 1, file);
		}

		struct collision_mesh *cm = &area->colmesh;
		for (u16 j = 0; j < 3; j++) {
			fread(cm->offset.v + j, 4, 1, file);
			cm->offset.v[j] += area->offset.v[j];
		}

		fread(&cm->triangle_count, 2, 1, file);
		cm->triangles =
			calloc(cm->triangle_count, sizeof(*cm->triangles));
		for (u16 j = 0; j < cm->triangle_count; j++) {
			struct collision_triangle *tri = cm->triangles + j;
			for (u8 k = 0; k < 3; k++) {
				for (u8 l = 0; l < 3; l++) {
					fread(tri->verts[k].pos + l, 4, 1,
					      file);
				}
			}

			for (u8 k = 0; k < 3; k++) {
				fread(tri->norm + k, 4, 1, file);
			}
		}

		fread(&area->actor_header_count, 2, 1, file);
		area->actor_headers = calloc(area->actor_header_count,
					     sizeof(*area->actor_headers));
		for (u16 j = 0; j < area->actor_header_count; j++) {
			/* FIXME: type_index is possible unused */
			u8 type, type_index;
			fread(&type, 1, 1, file);
			fread(&type_index, 1, 1, file);

			struct actor_header **actor_ptr =
				area->actor_headers + j;
			switch (type) {
			case ACTOR_TYPE_STATIC:
				*actor_ptr = actor_static_init(file);
				break;

			case ACTOR_TYPE_DOOR:
				*actor_ptr = actor_door_init(i, file);
				break;

			case ACTOR_TYPE_MICROWAVE:
				*actor_ptr = actor_microwave_init();
				break;

			case ACTOR_TYPE_PICKUP:
				*actor_ptr = actor_pickup_init(file);
				break;
			}

			struct actor_header *actor = *actor_ptr;
			actor->matrix = malloc_uncached(sizeof(*actor->matrix));

			rspq_block_begin();
			t3d_matrix_push(actor->matrix);
			t3d_model_draw(actor->mdl);
			t3d_matrix_pop(1);
			actor->displaylist = rspq_block_end();

			for (u8 k = 0; k < 3; k++) {
				fread(actor->position.v + k, 4, 1, file);
				actor->position.v[k] += area->offset.v[k];
			}

			for (u8 k = 0; k < 3; k++) {
				fread(actor->scale.v + k, 4, 1, file);
			}

			for (u8 k = 0; k < 4; k++) {
				fread(actor->rotation.v + k, 4, 1, file);
			}

			actor->position_old = actor->position;
			actor->scale_old = actor->scale;
			actor->rotation_old = actor->rotation;
			actor->position_init = actor->position;
			actor->scale_init = actor->scale;
			actor->rotation_init = actor->rotation;
			actor->flags = ACTOR_FLAG_IS_ACTIVE;
		}

		char colmesh_name[32];
		memset(colmesh_name, 0, 32);
		snprintf(colmesh_name, 32, "Col.%u", i);
		area->matrix = malloc_uncached(sizeof(*area->matrix));
		t3d_mat4fp_from_srt_euler(area->matrix, (f32[3]){ 1, 1, 1 },
					  (f32[3]){ 0, 0, 0 }, area->offset.v);

		rspq_block_begin();
		t3d_matrix_push(area->matrix);
		T3DModelIter iter =
			t3d_model_iter_create(scn.mdl, T3D_CHUNK_TYPE_OBJECT);
		while (t3d_model_iter_next(&iter)) {
			if (strncmp(iter.object->name, colmesh_name, 32)) {
				continue;
			}
			t3d_model_draw_material(iter.object->material, NULL);
			t3d_model_draw_object(iter.object, NULL);
		}
		t3d_matrix_pop(1);
		area->displaylist = rspq_block_end();
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
				     const T3DVec3 *player_dir, const f32 dt)
{
	if (!(actor->flags & ACTOR_FLAG_IS_ACTIVE) ||
	    (actor->flags & ACTOR_FLAG_WAS_UPDATED_THIS_FRAME)) {
		return;
	}

	switch (actor_update(actor, player_pos, player_dir, dt)) {
	case ACTOR_RETURN_LOAD_NEXT_AREA: {
		struct actor_door *door = actor_doors + actor->type_index;
		scn->area_index_old = scn->area_index;
		scn->area_index = door->area_dest;
		scn->flags |= SCENE_FLAG_PROCESS_AREA_LAST;
		struct actor_door *door_new = actor_door_find_by_area_dest(
			scn->area_index_old, scn->area_index);
		((struct actor_header *)door_new)->flags &=
			~(ACTOR_FLAG_IS_ACTIVE);
		return;
	}

	case ACTOR_RETURN_UNLOAD_PREV_AREA: {
		scn->flags &= ~(SCENE_FLAG_PROCESS_AREA_LAST);
		struct actor_door *door_cur = actor_door_find_by_area_dest(
			scn->area_index_old, scn->area_index);
		((struct actor_header *)door_cur)->flags |=
			(ACTOR_FLAG_IS_ACTIVE);
		return;
	}

	case ACTOR_RETURN_UNLOAD_NEXT_AREA:
		scn->flags &= ~(SCENE_FLAG_PROCESS_AREA_LAST);
		scn->area_index = scn->area_index_old;
		return;

	case ACTOR_RETURN_PICKUP_WEAPON: {
		struct actor_pickup *pu = actor_pickups + actor->type_index;
		struct inventory_entry *ent = player.inventory.entries +
					      player.inventory.entry_count++;
		ent->type = INV_ENT_TYPE_SHOTGUN;
		((struct actor_header *)pu)->flags &= ~(ACTOR_FLAG_IS_ACTIVE);
		debugf("Added Shotgun to inventory\n");
		player.inv_ent_cur = player.inventory.entry_count - 1;
		return;
	}

	default:
		return;
	}
}

void scene_update(struct scene *scn, const f32 dt)
{
	T3DVec3 player_eye, player_focus, player_dir;
	player_look_values_get(&player_eye, &player_focus, 1.f);
	t3d_vec3_diff(&player_dir, &player_focus, &player_eye);

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
						 &player.pos, &player_dir, dt);
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
