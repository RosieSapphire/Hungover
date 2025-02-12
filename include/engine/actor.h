#ifndef _ENGINE_ACTOR_H_
#define _ENGINE_ACTOR_H_

#ifndef IS_USING_SCENE_CONVERTER
#include "types.h"

#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>
#else
#define ACTOR_NAME_MAX_LEN 32
#endif

enum {
	ACTOR_TYPE_STATIC,
	ACTOR_TYPE_DOOR,
	ACTOR_TYPE_MICROWAVE,
	ACTOR_TYPE_COUNT
};

enum {
	ACTOR_RETURN_NONE,
	ACTOR_RETURN_LOAD_NEXT_AREA,
	ACTOR_RETURN_UNLOAD_PREV_AREA,
	ACTOR_RETURN_UNLOAD_NEXT_AREA,
	ACTOR_RETURN_COUNT
};

enum {
	ACTOR_FLAG_IS_ACTIVE = (1 << 0),
	ACTOR_FLAG_MUST_REENTER_RADIUS = (1 << 1),
	ACTOR_FLAG_WAS_UPDATED_THIS_FRAME = (1 << 2)
};

struct actor_header {
#ifndef IS_USING_SCENE_CONVERTER
	T3DModel *mdl;
	T3DMat4FP *matrix;
	rspq_block_t *displaylist;
#else
	char name[ACTOR_NAME_MAX_LEN];
#endif
	T3DVec3 position;
	T3DVec3 position_old;
	T3DVec3 position_init;
	T3DVec3 scale;
	T3DVec3 scale_old;
	T3DVec3 scale_init;
	T3DQuat rotation;
	T3DQuat rotation_old;
	T3DQuat rotation_init;
	u8 type;
	u8 type_index;
	u8 flags;
};

#ifndef IS_USING_SCENE_CONVERTER
struct actor_header *actor_init_from_file(FILE *file, const T3DVec3 *offset,
					  const u16 area_index);
void actor_static_vars_setup(void);
void actor_static_vars_to_ui(void);
u8 actor_update(struct actor_header *act, const T3DVec3 *player_pos,
		const T3DVec3 *player_dir, const f32 dt);
void actor_render(const struct actor_header *act);
/*
rspq_block_t *actorsInstancedGenDL(const u8 numstruct actor_headers,
				   struct actor_header *acts,
				   const T3DModel *commonModel);
				   */
void actor_matrix_setup(struct actor_header *act, const f32 subtick);
void actor_free(struct actor_header *act, const boolean should_free_model);
#endif

#endif /* _ENGINE_ACTOR_H_ */
