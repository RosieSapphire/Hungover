#ifndef ENGINE_PLAYER_H_
#define ENGINE_PLAYER_H_

#include "engine/update.h"
#include "engine/camera.h"

typedef struct {
	camera_t cam;
	float vel[3];
} player_t;

void player_init(player_t *p);
void player_update(player_t *p, update_parms_t uparms);
void player_view_matrix_setup(const player_t *p, float subtick);

#endif /* ENGINE_PLAYER_H_ */
