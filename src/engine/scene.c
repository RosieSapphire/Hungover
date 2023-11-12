#include <stdio.h>
#include <malloc.h>
#include <GL/gl.h>

#include "engine/util.h"
#include "engine/scene.h"

enum scene_index scene_index = SCENE_TESTROOM;
u32 pickup_spin_frame, pickup_spin_frame_last;

void scene_update(scene_t *s)
{
	for (int i = 0; i < s->num_anims; i++)
		animation_update(s->anims + i);

	pickup_spin_frame_last = pickup_spin_frame;
	pickup_spin_frame++;
}
