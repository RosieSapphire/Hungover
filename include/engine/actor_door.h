#ifndef _ENGINE_ACTOR_DOOR_H_
#define _ENGINE_ACTOR_DOOR_H_

#include "engine/actor.h"

#define ACTOR_DOOR_MAX_COUNT 16

struct actor_door {
	struct actor_header header;
	boolean is_opening;
	u8 side_entered;
	u16 area_index;
	u16 area_next;
	f32 swing_amount, swing_amount_old;
};

extern u8 actor_door_count;
extern u8 actor_door_count_in_range;
extern struct actor_door actor_doors[ACTOR_DOOR_MAX_COUNT];

struct actor_header *actor_door_init(const u16 area_next, const u16 area_index);
struct actor_door *actor_door_find_by_area_next(const u16 area_next);
u8 actor_door_update(struct actor_door *door,
		     const T3DVec3 *player_to_actor_dir, const f32 player_dist,
		     const f32 player_to_actor_dot, const f32 dt);
void actor_door_free(struct actor_door *door);

#endif /* _ENGINE_ACTOR_DOOR_H_ */
