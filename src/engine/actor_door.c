#include "input.h"
#include "t3d_ext.h"

#include "engine/actor_door.h"

#define DOOR_ACTOR_TURN_DEG_PER_SEC 179.f
#define DOOR_ACTOR_CHECK_RADIUS 100.f

u8 actor_door_count = 0;
u8 actor_door_count_in_range = 0;
struct actor_door actor_doors[ACTOR_DOOR_MAX_COUNT];

struct actor_header *actor_door_init(const u16 area_dest, const u16 area_index)
{
	const u8 ind = actor_door_count++;
	struct actor_door *door = actor_doors + ind;
	struct actor_header *head = (struct actor_header *)door;

	door->area_index = area_index;
	door->area_dest = area_dest;
	door->is_opening = false;
	door->side_entered = -1;
	door->swing_amount = 0.f;
	door->swing_amount_old = 0.f;

	head->type = ACTOR_TYPE_DOOR;
	head->type_index = ind;

	return head;
}

struct actor_door *actor_door_find_by_area_dest(const u16 door_area_dest,
						const u16 door_area_in)
{
	for (u16 i = 0; i < actor_door_count; i++) {
		struct actor_door *d = actor_doors + i;

		if (d->area_index != door_area_in) {
			continue;
		}

		if (d->area_dest != door_area_dest) {
			continue;
		}

		return d;
	}

	return NULL;
}

static void _actor_door_update_inside_range(struct actor_door *door,
					    const f32 facing_dot)
{
	if (facing_dot < 0.6f) {
		return;
	}

	actor_door_count_in_range++;
	if (INPUT_GET_BTN(A, PRESSED)) {
		door->is_opening ^= door->swing_amount == 0.f ||
				    door->swing_amount == 90.f;
	}
}

static void _actor_door_update_outside_range(struct actor_door *door)
{
	door->is_opening = false;
}

/*
 * Returns what scene action to perform depending on which
 * side the player is on. If the player doesn't change sides,
 * it unloads the next room, and if they *did* change sides,
 * the previous room would be unloaded.
 */
static u8
_actor_door_player_side_determine(struct actor_door *door,
				  const struct actor_update_params *params)
{
	T3DVec3 actor_to_player_dir;
	f32 pass_door_dot;

	t3d_vec3_negate(&actor_to_player_dir, params->player_to_actor_dir);
	pass_door_dot = t3d_vec3_dot(&actor_to_player_dir, &T3D_VEC3_XUP);

	/* door just opened */
	if (door->swing_amount > 0.f && door->swing_amount_old <= 0.f) {
		door->side_entered = pass_door_dot >= 0.f;
		return ACTOR_RETURN_LOAD_NEXT_AREA;
	}

	/* door just closed */
	if (door->swing_amount <= 0.f && door->swing_amount_old > 0.f) {
		u8 side_old = door->side_entered;
		door->side_entered = pass_door_dot >= 0.f;

		/* we have moved to the other side of the door */
		if (side_old ^ door->side_entered) {
			return ACTOR_RETURN_UNLOAD_PREV_AREA;
		}

		/* ... we have not */
		return ACTOR_RETURN_UNLOAD_NEXT_AREA;
	}

	return ACTOR_RETURN_NONE;
}

u8 actor_door_update(const u8 index, const struct actor_update_params *params)
{
	struct actor_door *door = actor_doors + index;

	door->swing_amount_old = door->swing_amount;

	const f32 player_to_actor_dot =
		t3d_vec3_dot(params->player_dir, params->player_to_actor_dir);
	if (params->player_dist < DOOR_ACTOR_CHECK_RADIUS) {
		_actor_door_update_inside_range(door, player_to_actor_dot);
	} else {
		_actor_door_update_outside_range(door);
	}

	if (door->is_opening) {
		door->swing_amount += params->dt * DOOR_ACTOR_TURN_DEG_PER_SEC;
		if (door->swing_amount > 90.f) {
			door->swing_amount = 90.f;
		}
	} else {
		door->swing_amount -= params->dt * DOOR_ACTOR_TURN_DEG_PER_SEC;
		if (door->swing_amount < 0.f) {
			door->swing_amount = 0.f;
		}
	}

	struct actor_header *head = (struct actor_header *)door;
	head->rotation = head->rotation_init;
	t3d_quat_rotate_euler(&head->rotation, (f32[3]){ 0, 0, 1 },
			      T3D_DEG_TO_RAD(door->swing_amount));

	/*
	 * TODO/FIXME: Change the logic for this to, instead of being
	 * the dot product of a perpendicular vector, two hitboxes
	 * on either side of the door (which can be determined by
	 * the vector in question. :3
	 */
	return _actor_door_player_side_determine(door, params);
}

void actor_door_free(struct actor_door *door)
{
	door->swing_amount = door->swing_amount_old = 0.f;
	door->side_entered = -1;
	door->is_opening = false;
	door->area_dest = -1;
	door->area_index = -1;
	actor_door_count--;
}
