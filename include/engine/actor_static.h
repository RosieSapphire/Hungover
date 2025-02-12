#ifndef _ENGINE_ACTOR_STATIC_H_
#define _ENGINE_ACTOR_STATIC_H_

#include "engine/actor.h"

#define ACTOR_STATIC_MAX_COUNT 32

struct actor_static {
	struct actor_header header;
};

extern u8 actor_static_count;
extern u8 actor_static_count_in_range;
extern struct actor_static actor_statics[ACTOR_STATIC_MAX_COUNT];

struct actor_header *actor_static_init(void);

#endif /* _ENGINE_ACTOR_STATIC_H_ */
