#include "engine/util.h"
#include "engine/vector.h"
#include "engine/texture.h"

#include "game/title.h"

static s32 opt_last_frame;
static int opt_selected_last;

void title_menu_option_change(struct update_parms uparms, s8 *opt_selected,
			      u8 music_state, u32 frame)
{
	static bool stick_menu_option_change;
	s8 os = *opt_selected;

	opt_selected_last = os;
	if (music_state == 2)
	{
		/* buttons */
		os += (uparms.down.c_down || uparms.down.d_down);
		os -= (uparms.down.c_up || uparms.down.d_up);

		/* Joystick */
		f32 stick_y = (f32)uparms.stick.stick_y / -85.0f;

		if (fabsf(stick_y) >= 0.5f)
		{
			if (!stick_menu_option_change)
			{
				stick_menu_option_change = true;

				if (stick_y > 0)
					os++;
				else
					os--;
			}
		}
		else
		{
			stick_menu_option_change = false;
		}
	}

	if (os > 2)
		os = 0;
	if (os < 0)
		os = 2;
	if (opt_selected_last != os)
		opt_last_frame = frame;

	*opt_selected = os;
}

void title_menu_option_draw_main(f32 t, s8 move_dir, u8 os,
				 u8 ind, f32 music_t)
{
	f32 time_last_select = (f32)((t * 12) - opt_last_frame) / CONF_TICKRATE;
	f32 select_offset = lerpf(-1.5f, -0.5f,
			   time_last_select * 3 * time_last_select * 3);
	f32 select_offset_bob = lerpf(-1.5f,
			       -0.5f + (sinf(time_last_select * 8) / 8.0f),
			       time_last_select * 3 * time_last_select * 3);
	f32 col = select_offset + 1.5f;
	f32 a[3], b[3], c[3];

	a[0] = 20 * move_dir;
	a[1] = -10;
	a[2] = -5.5f;

	b[0] = 0;
	b[1] = -ind;

	if (os == ind)
	{
		glColor3f(col, col, col);
		b[2] = select_offset_bob;
	}
	else
	{
		col = (1.0f - col) * 0.5f + 0.5f;
		glColor3f(col, col, col);
		b[2] = 1.0f / select_offset_bob;
	}
	vector_lerp(a, b, music_t, c, 3);
	glTranslatef(c[0], c[1], c[2]);
}

void title_menu_option_draw(const smesh_t *mesh, s8 move_dir, u8 music_state,
			    u8 os, f32 music_t, f32 t, int ind, u32 tex)
{
	f32 a[3], b[3], c[3];

	glLoadIdentity();
	switch (music_state)
	{
	case 0:
	case 1:
		break;

	case 2:
		title_menu_option_draw_main(t, move_dir, os, ind, music_t);
		break;

	case 3:
		/*
		 * _title_menu_option_draw_start();
		 */
		a[0] = 0;
		a[1] = -ind;
		a[2] = -1.5f;

		b[0] = 10 * move_dir;
		b[1] = 5;
		b[2] = 2.5f;

		vector_lerp(a, b, music_t, c, 3);
		glTranslatef(c[0], c[1], c[2]);
		break;
	}

	f32 to = t + (8 * ind);

	glTranslatef(sinf(to) * 0.04f, sinf(to * 1.5f) * 0.04f, 0);
	glRotatef(sinf(to) * 4, 0, 0, 1);
	smesh_draw_tex(mesh, tex);
}
