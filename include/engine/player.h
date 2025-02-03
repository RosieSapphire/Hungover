#ifndef _ENGINE_PLAYER_H_
#define _ENGINE_PLAYER_H_

#include "engine/camera.h"

typedef struct {
	camera_t cam;
	T3DVec3 pos, pos_old;
} player_t;

player_t player_init(void);
void player_update(player_t *p, const float dt);
void player_terminate(player_t *p);

#endif /* _ENGINE_PLAYER_H_ */
