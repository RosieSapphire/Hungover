#ifndef _ENGINE_AREA_H_
#define _ENGINE_AREA_H_

#include <stdio.h>

#ifndef IS_USING_SCENE_CONVERTER
#include "types.h"

#include <t3d/t3dmodel.h>
#endif

#include "engine/actor.h"
#include "engine/collision.h"

struct area {
	T3DVec3 offset;
	struct collision_mesh colmesh;
	u16 actor_header_count;
	struct actor_header *actor_headers;
#ifndef IS_USING_SCENE_CONVERTER
	rspq_block_t *displaylist;
	T3DMat4FP *matrix;
#endif
};

#ifndef IS_USING_SCENE_CONVERTER
struct area area_init_from_file(FILE *file, T3DModel *scene_model,
				const u16 index);
void area_render(const struct area *a, const f32 subtick);
void area_free(struct area *a);
#endif

#endif /* _ENGINE_AREA_H_ */
