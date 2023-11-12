#include <stdio.h>
#include <malloc.h>
#include <GL/gl.h>

#include "engine/util.h"
#include "engine/scene.h"

enum scene_index scene_index = SCENE_TITLE;
u32 pickup_spin_frame, pickup_spin_frame_last;

/**
 * scene_update - Updates all animations in a Scene
 * @s: Scene in Question
 */
void scene_update(struct scene *s)
{
	for (int i = 0; i < s->num_anims; i++)
		animation_update(s->anims + i);

	pickup_spin_frame_last = pickup_spin_frame;
	pickup_spin_frame++;
}
