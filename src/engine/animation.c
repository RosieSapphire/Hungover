#include <GL/gl.h>
#include <libdragon.h>

#include "engine/util.h"
#include "engine/vector.h"
#include "engine/animation.h"

void animation_update(animation_t *a)
{
	a->frame_last = a->frame;

	if(a->is_backward)
		a->frame--;
	else
		a->frame++;

	if(a->loops) {
		if((int16_t)a->frame < 0)
			a->frame = a->length - 1;

		if(a->frame >= a->length)
			a->frame = 0;

		return;
	}

	a->frame = clampi(a->frame, 0, a->length - 1);
}

void animation_setup_matrix(__attribute__((unused))const animation_t *a, __attribute__((unused))float subtick)
{
	float pos_lerp[3] = {0, 0, 0};
	float rot_lerp[4] = {0, 0, 0, 1};
	float sca_lerp[3] = {1, 1, 1};

	if(a->num_pos) {
		const float *pos_last =
			a->pos[clampi(a->frame_last, 0, a->num_pos - 1)].vec;
		const float *pos_cur =
			a->pos[clampi(a->frame, 0, a->num_pos - 1)].vec;
		vector_lerp(pos_last, pos_cur, subtick, pos_lerp, 3);
	}

	if(a->num_rot) {
		const float *rot_last =
			a->rot[clampi(a->frame_last, 0, a->num_rot - 1)].vec;
		const float *rot_cur =
			a->rot[clampi(a->frame, 0, a->num_rot - 1)].vec;
		quat_lerp(rot_last, rot_cur, rot_lerp, subtick);
	}

	if(a->num_sca) {
		const float *sca_last =
			a->sca[clampi(a->frame_last, 0, a->num_sca - 1)].vec;
		const float *sca_cur =
			a->sca[clampi(a->frame, 0, a->num_sca - 1)].vec;
		vector_lerp(sca_last, sca_cur, subtick, sca_lerp, 3);
	}
	
	float mat[4 * 4] = {0};
	mat[0] = (1 - 2 * rot_lerp[1] * rot_lerp[1] - 2 *
			rot_lerp[2] * rot_lerp[2]) * sca_lerp[0];
	mat[1] = (2 * rot_lerp[0] * rot_lerp[1] + 2 *
			rot_lerp[2] * rot_lerp[3]) * sca_lerp[0];
	mat[2] = (2 * rot_lerp[0] * rot_lerp[2] - 2 *
			rot_lerp[1]*rot_lerp[3]) * sca_lerp[0];
	mat[3] = 0.0f;
	
	mat[4] = (2 * rot_lerp[0] * rot_lerp[1] - 2 *
			rot_lerp[2] * rot_lerp[3]) * sca_lerp[1];
	mat[5] = (1 - 2 * rot_lerp[0] * rot_lerp[0] - 2 *
			rot_lerp[2] * rot_lerp[2]) * sca_lerp[1];
	mat[6] = (2 * rot_lerp[1] * rot_lerp[2] + 2 *
			rot_lerp[0] * rot_lerp[3]) * sca_lerp[1];
	mat[7] = 0.0f;
	
	mat[8] = (2 * rot_lerp[0] * rot_lerp[2] + 2 *
			rot_lerp[1]*rot_lerp[3]) * sca_lerp[2];
	mat[9] = (2 * rot_lerp[1] * rot_lerp[2] - 2 *
			rot_lerp[0] * rot_lerp[3]) * sca_lerp[2];
	mat[10] = (1 - 2 * rot_lerp[0] * rot_lerp[0] - 2 *
			rot_lerp[1]*rot_lerp[1]) * sca_lerp[2];
	mat[11] = 0.0f;
	
	mat[12] = pos_lerp[0];
	mat[13] = pos_lerp[1];
	mat[14] = pos_lerp[2];
	mat[15] = 1.0f;

	glMultMatrixf(mat);
}

void animation_destroy(animation_t *a)
{
	a->mesh_index = a->length = a->num_pos =
		a->num_rot = a->num_sca = a->frame = a->is_playing = 0;
	free(a->pos);
	free(a->rot);
	free(a->sca);
}
