#ifndef _ENGINE_PLAYER_H_
#define _ENGINE_PLAYER_H_

#include <t3d/t3d.h>

#include "config.h"

#include "engine/collision.h"
#include "engine/scene.h"

#define PLAYER_NUM_COLLISION_MESH_PTRS 2

typedef struct {
	T3DVec3 pos, pos_old;
	float yaw, yaw_old, pitch, pitch_old;
	collision_mesh_t *collision_mesh_ptrs[PLAYER_NUM_COLLISION_MESH_PTRS];
} player_t;

player_t player_init(collision_mesh_t *colmesh_ptrs,
		     const int num_colmesh_ptrs);
void player_get_look_values(T3DVec3 *eye, T3DVec3 *focus, const player_t *p,
			    const float interp);
void player_update(player_t *p, const scene_t *scn, const float dt);
void player_to_viewport(T3DViewport *vp, const player_t *p, const float interp);
void player_terminate(player_t *p);

#endif /* _ENGINE_PLAYER_H_ */
