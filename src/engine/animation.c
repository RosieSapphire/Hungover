#include <GL/gl.h>
#include <libdragon.h>

#include "engine/vector.h"
#include "engine/animation.h"

void animation_debug(const animation_t *a)
{
	const float *pos_cur = a->sca[a->frame].vec;
	debugf("Anim '%s' : %3.d/%3.d\tsca=(%f, %f, %f)\n",
			a->name, a->frame, a->length,
			pos_cur[0], pos_cur[1], pos_cur[1]);
}

void animation_update(animation_t *a)
{
	a->frame_last = a->frame;
	a->frame++;
	a->frame %= a->length + 1;
}

static uint16_t _animation_get_frame_last(const animation_t *a)
{
	int16_t r = (int16_t)a->frame - 1;
	if(r < 0)
		r += a->length + 1;
	return r;
}

void animation_setup_matrix(const animation_t *a, float subtick)
{
	int frame_last = _animation_get_frame_last(a);
	const float *pos_last = a->pos[frame_last].vec;
	const float *pos_cur = a->pos[a->frame].vec;
	float pos_lerp[3];
	vector_lerp(pos_last, pos_cur, subtick, pos_lerp);

	const float *sca_last = a->sca[frame_last].vec;
	const float *sca_cur = a->sca[a->frame].vec;
	float sca_lerp[3];
	vector_lerp(sca_last, sca_cur, subtick, sca_lerp);

	glTranslatef(pos_lerp[0], pos_lerp[1], pos_lerp[2]);
	glScalef(sca_lerp[0], sca_lerp[1], sca_lerp[2]);
}
