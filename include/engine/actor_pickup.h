#ifndef _ENGINE_ACTOR_PICKUP_H_
#define _ENGINE_ACTOR_PICKUP_H_

#include <stdio.h>

#ifndef IS_USING_GLTF_TO_SCN
#include "engine/actor.h"
#else /* IS_USING_GLTF_TO_SCN */
#include "../../../include/engine/actor.h"
#endif /* IS_USING_GLTF_TO_SCN */

#define ACTOR_PICKUP_MAX_COUNT 128

enum { PICKUP_TYPE_SHOTGUN, PICKUP_TYPE_COUNT };

struct actor_pickup {
	struct actor_header header;
	u8 type;
};

extern u8 actor_pickup_count;
extern u8 actor_pickup_count_in_range;
extern struct actor_pickup actor_pickups[ACTOR_PICKUP_MAX_COUNT];

struct actor_header *actor_pickup_init(FILE *file);
u8 actor_pickup_update(const u8 index,
		       const struct actor_update_params *params);

#endif /* _ENGINE_ACTOR_PICKUP_H_ */
