#include <GL/gl.h>

#include "engine/util.h"
#include "engine/vector.h"

#include "game/title.h"

/**
 * title_logo_transform_intro - Transforms Title Logo for Intro
 * @beats_lerp: A pointer to the beats_lerp variable
 *
 * Description: Transforms the title logo for the intro music state
 * Return: Bool for whether or not to draw logo
 */
u8 title_logo_transform_intro(f32 *beats_lerp)
{
	f32 a[3] = {0, 0, -10};
	f32 b[3] = {0, 0, 0};
	f32 c[3];
	f32 bl = *beats_lerp;

	bl = clampf(bl - 29.5f, 0, 3) / 3.0f;

	if (bl <= 0.0f)
		return (0);

	vector_lerp(a, b, bl * bl, c, 3);
	glTranslatef(c[0], c[1], c[2]);
	glRotatef(bl * 720 * bl, 0, 0, 1);

	*beats_lerp = bl;
	return (1);
}

/**
 * title_logo_transform_main - Transforms Title Logo for Main Menu
 * @music_t: Music Interpolation
 * @music_ch_last: Last music channel played
 *
 * Description: Transforms the title logo for the main menu music state
 * Return: Bool for whether or not to draw logo
 */
u8 title_logo_transform_main(const f32 music_t, const u8 music_ch_last)
{
	f32 a[3] = {0,    0,     0};
	f32 b[3] = {0, 1.5f, -1.0f};
	f32 c[3];

	if (music_ch_last == 4)
		vector_copy(b, a, 3);

	vector_smooth(a, b, music_t, c, 3);
	glTranslatef(c[0], c[1], c[2]);

	return (1);
}

/**
 * title_logo_transform_start - Transforms Title Logo for Starting the Game
 * @beats_lerp: The subtick-interpolated beat counter value.
 * @difft: The difference of t
 * @music_state_last: Last Music State
 *
 * Description: Transforms the title logo for the game starting music state
 * Return: Bool for whether or not to draw logo
 */
u8 title_logo_transform_start(const f32 beats_lerp, f32 *difft,
			      const u8 music_state_last)
{
	static f32 beats_start = 0.0f;
	f32 dt = *difft;

	if (music_state_last == 2)
		beats_start = beats_lerp;

	f32 a[3] = {0, 1.5f, -1.0f};
	f32 b[3] = {0,    0,     0};
	f32 c[3];

	dt = (beats_lerp - beats_start) / 3.5f;
	dt *= dt * dt;
	dt = clampf(dt, 0, 1);
	vector_lerp(a, b, dt, c, 3);

	glTranslatef(c[0], c[1], c[2]);
	glRotatef(smoothf(0, 720, dt), 0, 0, 1);

	*difft = dt;

	return (1);
}

void title_logo_draw_object(const f32 difft, const u8 i, const u8 num_it,
			    const f32 beats_lerp, const f32 t,
			    const smesh_t *text_mesh, const u32 tid,
			    u8 *bg_is_white)
{
	if (difft >= 1.0f)
	{
		glColor3f(0, 0, 0);
		glTranslatef(0, 0, 1.2f);
		*bg_is_white = true;
		smesh_draw_tex(text_mesh, tid);
		return;
	}

	f32 z_trans;

	if (i != num_it - 1)
		z_trans = (i * 0.4f) - wrapf(beats_lerp * 0.125f, 0.4f);
	else
		z_trans = 1.2f;

	f32 ti = smoothf(t, t + i, (1.2f - z_trans) / 1.6f);
	f32 bright = (z_trans + 0.4f) / 1.6f;

	glTranslatef(sinf(ti) * 0.04f * (1.0f - difft),
	      sinf(ti * 1.5f) * 0.04f * (1.0f - difft), z_trans);
	glRotatef(sinf(ti) * 4 * (1.0f - difft), 0, 0, 1);
	glScalef(bright, bright, 1);
	bright *= bright * bright;
	glColor3f(bright, bright, bright);

	smesh_draw_tex(text_mesh, tid);
}

void title_logo_draw_press_start(const smesh_t *mesh, const u32 tid,
				 const u8 music_state, const f32 music_beats)
{
	if (music_state != 1)
		return;

	glLoadIdentity();
	glScalef(2, 1, 1);
	glTranslatef(0, -2.5f, -2);
	f32 blink = (music_beats * 0.25f) - (int)(music_beats * 0.25f);

	blink = (int)(blink * 2) & 1;
	glColor3f(blink, blink, blink);
	smesh_draw_tex(mesh, tid);
}
