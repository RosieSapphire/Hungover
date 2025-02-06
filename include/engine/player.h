#ifndef _ENGINE_PLAYER_H_
#define _ENGINE_PLAYER_H_

#include <t3d/t3d.h>

#include "config.h"

#include "engine/collision.h"

typedef struct {
	T3DVec3 pos, pos_old;
	float yaw, yaw_old, pitch, pitch_old;
	collision_mesh_t *collision_mesh_ptr;
} player_t;

player_t player_init(collision_mesh_t *colmesh_ptr);
void player_get_look_values(T3DVec3 *eye, T3DVec3 *focus, const player_t *p,
			    const float interp);
void player_update(player_t *p, const float dt);
void player_to_viewport(T3DViewport *vp, const player_t *p, const float interp);
void player_terminate(player_t *p);

#endif /* _ENGINE_PLAYER_H_ */
