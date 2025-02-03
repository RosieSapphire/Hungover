#include "util.h"
#include "t3d_ext.h"

#include "engine/player.h"

player_t player_init(void)
{
	player_t p;

	p.cam = camera_init(&T3DVEC3(0.f, 0.f, 0.f), 0.f, 0.f, NULL);

	return p;
}

void player_update(player_t *p, const float dt)
{
	camera_update(&p->cam, dt);
}

void player_terminate(player_t *p)
{
	camera_terminate(&p->cam);
	t3d_vec3_zero(&p->pos);
	t3d_vec3_zero(&p->pos_old);
}
