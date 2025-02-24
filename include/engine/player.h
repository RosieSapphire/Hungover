#ifndef _ENGINE_PLAYER_H_
#define _ENGINE_PLAYER_H_

#include <t3d/t3d.h>

#include "config.h"
#include "types.h"

#include "engine/collision.h"
#include "engine/scene.h"
#include "engine/inventory.h"

#define PLAYER_COLMESH_PTR_COUNT 2

struct player {
	T3DVec3 pos;
	T3DVec3 pos_old;
	f32 yaw;
	f32 yaw_old;
	f32 pitch;
	f32 pitch_old;
	struct collision_mesh *colmesh_ptrs[PLAYER_COLMESH_PTR_COUNT];
	struct inventory inventory;
	u8 inv_ent_cur;
};

extern struct player player;

void player_init(void);
void player_look_values_get(T3DVec3 *eye, T3DVec3 *focus, const f32 interp);
void player_update(const struct scene *scn, const f32 dt);
void player_to_viewport(T3DViewport *vp, const f32 interp);
void player_free(void);

#endif /* _ENGINE_PLAYER_H_ */
