#ifndef _ENGINE_AREA_H_
#define _ENGINE_AREA_H_

#include <stdio.h>

#ifdef IS_USING_GLTF_TO_SCN
#define AREA_NAME_MAX_LEN 64

#include "../../../include/types.h"

#include "../../../include/engine/actor.h"
#include "../../../include/engine/collision.h"
#else
#include "types.h"

#include <t3d/t3dmodel.h>

#include "engine/actor.h"
#include "engine/collision.h"
#endif

struct area {
	T3DVec3 offset;
	struct collision_mesh colmesh;
	u16 actor_header_count;
#ifndef IS_USING_GLTF_TO_SCN
	struct actor_header **actor_headers;
	T3DMat4FP *matrix;
	rspq_block_t *displaylist;
#else /* IS_USING_GLTF_TO_SCN */
	/*
	 * I need to use single pointers for the scene converter because
	 * otherwise it's a cocksucking motherfucking nightmare to work with.
	 */
	struct actor_header *actor_headers;
	char name[AREA_NAME_MAX_LEN];
#endif /* IS_USING_GLTF_TO_SCN */
};

#ifndef IS_USING_GLTF_TO_SCN
void area_render(const struct area *a, const f32 subtick);
void area_free(struct area *a);
#endif

#endif /* _ENGINE_AREA_H_ */
