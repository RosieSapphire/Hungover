#include "t3d_ext.h"
#include "input.h"

#include "engine/actor_pickup.h"

#define ACTOR_PICKUP_RADIUS 90.f

u8 actor_pickup_count = 0;
u8 actor_pickup_count_in_range = 0;
struct actor_pickup actor_pickups[ACTOR_PICKUP_MAX_COUNT];

struct actor_header *actor_pickup_init(FILE *file)
{
	const u8 ind = actor_pickup_count++;
	struct actor_header *h = (struct actor_header *)(actor_pickups + ind);

	h->type = ACTOR_TYPE_PICKUP;
	h->type_index = ind;

	struct actor_pickup *pu = actor_pickups + ind;
	fread(&pu->type, 1, 1, file);

	switch (pu->type) {
	case PICKUP_TYPE_SHOTGUN:
		h->mdl = t3d_model_load("rom:/Act.Shotgun.t3dm");
		break;
	}

	return h;
}

u8 actor_pickup_update(const u8 index, const struct actor_update_params *params)
{
	if (params->player_dist >= ACTOR_PICKUP_RADIUS) {
		return ACTOR_RETURN_NONE;
	}

	T3DVec3 pdir = { { params->player_dir->v[0], params->player_dir->v[1],
			   0.f } };
	T3DVec3 ptadir = { { params->player_to_actor_dir->v[0],
			     params->player_to_actor_dir->v[1], 0.f } };
	t3d_vec3_norm(&pdir);
	t3d_vec3_norm(&ptadir);

	f32 pass_pickup_dot = t3d_vec3_dot(&pdir, &ptadir);
	if (pass_pickup_dot > 0.9f) {
		actor_pickup_count_in_range++;

		if (INPUT_GET_BTN(A, PRESSED)) {
			((struct actor_header *)(actor_pickups + index))
				->flags &= ~(ACTOR_FLAG_IS_ACTIVE);
			return ACTOR_RETURN_PICKUP_WEAPON;
		}

		return ACTOR_RETURN_NONE;
	}

	return ACTOR_RETURN_NONE;
}
