#ifndef _ENGINE_PLAYER_H_
#define _ENGINE_PLAYER_H_

#include "engine/camera.h"

typedef struct {
	camera_t cam;
} player_t;

player_t player_init(void);
void player_update(player_t *p, const float dt);

#endif /* _ENGINE_PLAYER_H_ */
