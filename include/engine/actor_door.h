#ifndef _ENGINE_ACTOR_DOOR_H_
#define _ENGINE_ACTOR_DOOR_H_

#ifndef IS_USING_GLTF_TO_SCN
#include "engine/actor.h"
#else /* IS_USING_GLTF_TO_SCN */
#include "../../include/engine/actor.h"
#endif /* IS_USING_GLTF_TO_SCN */

#define ACTOR_DOOR_MAX_COUNT 16

struct actor_door {
	struct actor_header header;
	boolean is_opening;
	u8 side_entered;
	u16 area_index;
	u16 area_dest;
	f32 swing_amount, swing_amount_old;
};

extern u8 actor_door_count;
extern u8 actor_door_count_in_range;
extern struct actor_door actor_doors[ACTOR_DOOR_MAX_COUNT];

struct actor_header *actor_door_init(const u16 area_dest, const u16 area_index);
struct actor_door *actor_door_find_by_area_dest(const u16 door_area_dest,
						const u16 door_area_in);
u8 actor_door_update(const u8 index, const struct actor_update_params *params);
void actor_door_free(struct actor_door *door);

#endif /* _ENGINE_ACTOR_DOOR_H_ */
