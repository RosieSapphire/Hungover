#include "util.h"

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
