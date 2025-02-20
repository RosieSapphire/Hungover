#ifndef _ENGINE_ACTOR_STATIC_H_
#define _ENGINE_ACTOR_STATIC_H_

#ifndef IS_USING_GLTF_TO_SCN
#include "engine/actor.h"
#else /* IS_USING_GLTF_TO_SCN */
#include "../../../include/engine/actor.h"
#endif /* IS_USING_GLTF_TO_SCN */

#define ACTOR_STATIC_MAX_COUNT 32
#define ACTOR_STATIC_MDLPATH_MAX_LEN 64

struct actor_static {
	struct actor_header header;
#ifdef IS_USING_GLTF_TO_SCN
	char mdl_path[ACTOR_STATIC_MDLPATH_MAX_LEN];
#endif /* IS_USING_GLTF_TO_SCN */
};

extern u8 actor_static_count;
extern u8 actor_static_count_in_range;
extern struct actor_static actor_statics[ACTOR_STATIC_MAX_COUNT];

struct actor_header *actor_static_init(void);
u8 actor_static_update(const u8 index,
		       const struct actor_update_params *params);

#endif /* _ENGINE_ACTOR_STATIC_H_ */
