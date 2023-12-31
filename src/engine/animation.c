#include <GL/gl.h>
#include <libdragon.h>

#include "engine/util.h"
#include "engine/vector.h"
#include "engine/animation.h"

/**
 * animation_update - Updates Animation
 * @a: Animation to Update
 */
void animation_update(struct animation *a)
{
	a->frame_last = a->frame;

	if (a->flags & ANIM_IS_BACKWARD)
		a->frame--;
	else
		a->frame++;

	if (a->flags & ANIM_IS_LOOPING)
	{
		if ((s16)a->frame < 0)
			a->frame = a->length - 1;

		if (a->frame >= a->length)
			a->frame = 0;

		return;
	}

	a->frame = clampi(a->frame, 0, a->length - 1);
}

/**
 * _animation_setup_values_lerp - Sets up Pos/Rot/Sca Lerp Values
 * @a: Animation to Update
 * @subtick: Subtick Between Frames
 * @pos_out: Position Output
 * @rot_out: Rotation Output
 * @sca_out: Scaling Output
 */
static void _animation_setup_values_lerp(const struct animation *a,
					 f32 subtick, f32 *pos_out,
					 f32 *rot_out, f32 *sca_out)
{
	if (a->num_pos)
	{
		const f32 *pos_last =
			a->pos[clampi(a->frame_last, 0, a->num_pos - 1)].vec;
		const f32 *pos_cur =
			a->pos[clampi(a->frame, 0, a->num_pos - 1)].vec;

		vector_lerp(pos_last, pos_cur, subtick, pos_out, 3);
	}

	if (a->num_rot)
	{
		const f32 *rot_last =
			a->rot[clampi(a->frame_last, 0, a->num_rot - 1)].vec;
		const f32 *rot_cur =
			a->rot[clampi(a->frame, 0, a->num_rot - 1)].vec;

		quat_lerp(rot_last, rot_cur, rot_out, subtick);
	}

	if (a->num_sca)
	{
		const f32 *sca_last =
			a->sca[clampi(a->frame_last, 0, a->num_sca - 1)].vec;
		const f32 *sca_cur =
			a->sca[clampi(a->frame, 0, a->num_sca - 1)].vec;

		vector_lerp(sca_last, sca_cur, subtick, sca_out, 3);
	}

}

/**
 * animation_setup_matrix - Sets up OpenGL Matrix for Animation
 * @a: Animation to Set up Matrix for
 * @subtick: Subtick Between Frames
 */
void animation_setup_matrix(const struct animation *a, f32 subtick)
{
	f32 pos_lerp[3] = {0, 0, 0};
	f32 rot_lerp[4] = {0, 0, 0, 1};
	f32 sca_lerp[3] = {1, 1, 1};

	_animation_setup_values_lerp(a, subtick, pos_lerp, rot_lerp, sca_lerp);

	f32 mat[4 * 4] = {0};

	mat[0] = (1 - 2 * rot_lerp[1] * rot_lerp[1] - 2 *
			rot_lerp[2] * rot_lerp[2]) * sca_lerp[0];
	mat[1] = (2 * rot_lerp[0] * rot_lerp[1] + 2 *
			rot_lerp[2] * rot_lerp[3]) * sca_lerp[0];
	mat[2] = (2 * rot_lerp[0] * rot_lerp[2] - 2 *
			rot_lerp[1] * rot_lerp[3]) * sca_lerp[0];
	mat[3] = 0.0f;

	mat[4] = (2 * rot_lerp[0] * rot_lerp[1] - 2 *
			rot_lerp[2] * rot_lerp[3]) * sca_lerp[1];
	mat[5] = (1 - 2 * rot_lerp[0] * rot_lerp[0] - 2 *
			rot_lerp[2] * rot_lerp[2]) * sca_lerp[1];
	mat[6] = (2 * rot_lerp[1] * rot_lerp[2] + 2 *
			rot_lerp[0] * rot_lerp[3]) * sca_lerp[1];
	mat[7] = 0.0f;

	mat[8] = (2 * rot_lerp[0] * rot_lerp[2] + 2 *
			rot_lerp[1] * rot_lerp[3]) * sca_lerp[2];
	mat[9] = (2 * rot_lerp[1] * rot_lerp[2] - 2 *
			rot_lerp[0] * rot_lerp[3]) * sca_lerp[2];
	mat[10] = (1 - 2 * rot_lerp[0] * rot_lerp[0] - 2 *
			rot_lerp[1] * rot_lerp[1]) * sca_lerp[2];
	mat[11] = 0.0f;

	mat[12] = pos_lerp[0];
	mat[13] = pos_lerp[1];
	mat[14] = pos_lerp[2];
	mat[15] = 1.0f;

	glMultMatrixf(mat);
}

/**
 * animation_destroy - Destroys an Animation
 * @a: Animation to Destroy
 */
void animation_destroy(struct animation *a)
{
	a->mesh_index = a->length = a->num_pos =
		a->num_rot = a->num_sca = a->frame = 0;
	a->flags = ANIM_FLAGS_NONE;
	free(a->pos);
	free(a->rot);
	free(a->sca);
}
