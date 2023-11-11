#include <GL/gl.h>
#include <GL/gl_integration.h>

#include "engine/sfx.h"
#include "engine/config.h"
#include "engine/util.h"
#include "engine/vector.h"
#include "engine/texture.h"

#include "game/title.h"

#define MUSIC_bPM 178
#define MUSIC_DELTA ((MUSIC_bPM / 60.0f) / (f32)CONF_TICKRATE)

static bool is_loaded;
static long frame_counter;

static int music_state_last;
static int music_state;
static f32 music_beats_last;
static f32 music_beats;
static f32 music_state_timer_last;
static f32 music_state_timer;
static int music_ch_last;
static int music_ch_cur;

static int option_last_frame;
static int option_selected_last;
static int option_selected;

static texture_t hungover_text;
static texture_t new_game_text;
static texture_t continue_text;
static texture_t options_text;
static texture_t press_start_text;

static bool bg_is_white;

static smesh_t *text_mesh;

void _title_load(void)
{
	if (is_loaded)
		return;

	texture_create_file("rom:/title_text.ia8.sprite");
	texture_create_file("rom:/new_game.ia8.sprite");
	texture_create_file("rom:/continue.ia8.sprite");
	texture_create_file("rom:/options.ia8.sprite");
	texture_create_file("rom:/press_start.ia8.sprite");

	hungover_text    = tex_objs_loaded[0];
	new_game_text    = tex_objs_loaded[1];
	continue_text    = tex_objs_loaded[2];
	options_text     = tex_objs_loaded[3];
	press_start_text = tex_objs_loaded[4];

	frame_counter = 0;
	option_last_frame = 0;
	music_state_last = 0;
	music_state = 0;
	music_beats_last = 0.0f;
	music_beats = music_beats_last;
	music_state_timer_last = 1.0f;
	music_state_timer = music_state_timer_last;
	music_ch_last = SFXC_MUSIC0;
	music_ch_cur = SFXC_MUSIC0;

	projection_setup();

	const vertex_t verts[4] = {
		{{-1.0f, -0.5f, -2.5f}, {0.0f, 1.0f}},
		{{ 1.0f, -0.5f, -2.5f}, {1.0f, 1.0f}},
		{{-1.0f,  0.5f, -2.5f}, {0.0f, 0.0f}},
		{{ 1.0f,  0.5f, -2.5f}, {1.0f, 0.0f}},
	};

	const u16 indis[6] = {0, 1, 2, 2, 1, 3};

	text_mesh = smesh_create_data("UI", 4, 6, verts, indis);
	is_loaded = true;
}

void _title_unload(void)
{
	if (!is_loaded)
		return;

	texture_destroy(&press_start_text);
	texture_destroy(&options_text);
	texture_destroy(&continue_text);
	texture_destroy(&new_game_text);
	texture_destroy(&hungover_text);

	is_loaded = false;
}

static void _title_option_change(struct update_parms uparms)
{
	static bool stick_option_change;

	option_selected_last = option_selected;
	if (music_state == 2)
	{
		/* buttons */
		option_selected += (uparms.down.c_down || uparms.down.d_down);
		option_selected -= (uparms.down.c_up || uparms.down.d_up);

		/* Joystick */
		f32 stick_y = (f32)uparms.stick.stick_y / -85.0f;

		if (fabsf(stick_y) >= 0.5f)
		{
			if (!stick_option_change)
			{
				stick_option_change = true;

				if (stick_y > 0)
					option_selected++;
				else
					option_selected--;
			}
		}
		else
		{
			stick_option_change = false;
		}
	}

	if (option_selected > 2)
		option_selected = 0;
	if (option_selected < 0)
		option_selected = 2;

	if (option_selected_last != option_selected)
		option_last_frame = frame_counter;

}

static void _title_music_change(struct update_parms uparms)
{
	music_state_last = music_state;
	switch (music_state)
	{
	case 0:
	case 1:
		music_state += uparms.down.start;
		break;

	case 2:
		switch (option_selected)
		{
		case 0:
		case 1:
			music_state += uparms.down.start || uparms.down.a;
			break;

		case 2:
			music_state += uparms.down.start || uparms.down.a;
			break;
		}
		break;

	case 3:
		break;

	case 4:
		music_state -= uparms.down.b * 2;
		break;
	}
}

void _title_music_update(void)
{
	music_beats_last = music_beats;
	music_beats += MUSIC_DELTA;
	if (music_beats >= 33.0f && music_state == 0)
		music_state = 1;

	if (music_state_last != music_state)
	{
		music_ch_last = music_ch_cur;
		music_ch_cur = music_state;
		music_state_timer = 0.0f;

		if (music_ch_cur == SFXC_MUSIC3)
		{
			mixer_ch_stop(SFXC_MUSIC0);
			mixer_ch_stop(SFXC_MUSIC1);
			mixer_ch_stop(SFXC_MUSIC2);
			mixer_ch_stop(SFXC_MUSIC4);
			wav64_play(&title_music3, SFXC_MUSIC3);
		}
	}

	music_state_timer_last = music_state_timer;
	music_state_timer += 0.06f;
}

enum scene_index title_update(struct update_parms uparms)
{
	frame_counter++;
	_title_music_change(uparms);
	_title_music_update();

	if (music_state == 3 && music_state_timer >= 4.0f)
	{
		_title_unload();
		return (SCENE_TESTROOM);
	}

	_title_option_change(uparms);

	return (SCENE_TITLE);
}

static u8 _title_logo_setup_transform(f32 *beats_lerp,
					f32 *difft, f32 music_t)
{
	f32 a[3], b[3], c[3];
	f32 bl = *beats_lerp;
	f32 dt = *difft;

	switch (music_state)
	{
	case 0:
		a[0] =   0;
		a[1] =   0;
		a[2] = -10;

		b[0] = 0;
		b[1] = 0;
		b[2] = 0;

		bl = clampf(bl - 29.5f, 0, 3) / 3.0f;
		debugf("%f\n", bl);

		if (bl <= 0.0f)
			return (0);

		vector_lerp(a, b, bl * bl, c, 3);
		glTranslatef(c[0], c[1], c[2]);
		glRotatef(bl * 720 * bl, 0, 0, 1);
		break;

	case 1:
		break;

	case 2:
		f32 a[3] = {0,    0,     0};
		f32 b[3] = {0, 1.5f, -1.0f};

		if (music_ch_last == 4)
			vector_copy(b, a, 3);

		vector_smooth(a, b, music_t, c, 3);
		glTranslatef(c[0], c[1], c[2]);

		break;

	case 3:
		static f32 beats_start = 0.0f;

		if (music_state_last == 2)
			beats_start = bl;

		a[0] = 0.0f;
		a[1] = 1.5f;
		a[2] = -1.0f;

		b[0] = 0.0f;
		b[1] = 0.0f;
		b[2] =  0.0f;

		dt = (bl - beats_start) / 3.5f;
		dt *= dt * dt;
		dt = clampf(dt, 0, 1);
		vector_lerp(a, b, dt, c, 3);

		glTranslatef(c[0], c[1], c[2]);
		glRotatef(smoothf(0, 720, dt), 0, 0, 1);
		break;

	case 4:
		glTranslatef(0, 1.5f, -1.0f);
		break;
	}

	*beats_lerp = bl;
	*difft = dt;

	return (1);
}

static void _title_logo_draw(f32 music_t, f32 t, f32 subtick)
{
	int num_it = 5;

	for (int i = 0; i < num_it; i++)
	{
		glLoadIdentity();
		f32 beats_lerp = lerpf(music_beats_last, music_beats, subtick);
		f32 difft = 0.0f;

		if(!_title_logo_setup_transform(&beats_lerp, &difft, music_t))
			break;

		bool only_one = difft >= 1.0f;

		if (only_one)
		{
			glColor3f(0, 0, 0);
			glTranslatef(0, 0, 1.2f);
			bg_is_white = true;
		}
		else
		{
			f32 z_trans;

			if (i != num_it - 1)
				z_trans = (i * 0.4f) -
					wrapf(beats_lerp * 0.125f, 0.4f);
			else
				z_trans = 1.2f;

			f32 ti = smoothf(t, t + i, (1.2f - z_trans) / 1.6f);
			f32 bright = (z_trans + 0.4f) / 1.6f;

			glTranslatef(sinf(ti) * 0.04f * (1.0f - difft),
					sinf(ti * 1.5f) * 0.04f * (1.0f - difft),
					z_trans);
			glRotatef(sinf(ti) * 4 * (1.0f - difft), 0, 0, 1);
			glScalef(bright, bright, 1);
			bright *= bright * bright;
			glColor3f(bright, bright, bright);
		}

		smesh_draw_tex(text_mesh, hungover_text.id);

		if (only_one)
			return;
	}
}

static void _title_menu_option_draw_main(f32 t, s8 move_dir,
					 u8 ind, f32 music_t)
{
	f32 time_last_select =
		(f32)((t * 12) - option_last_frame) / CONF_TICKRATE;
	f32 select_offset = lerpf(-1.5f, -0.5f,
			   time_last_select * 3 * time_last_select * 3);
	f32 select_offset_bob =
		lerpf(-1.5f, -0.5f + (sinf(time_last_select * 8) / 8.0f),
		time_last_select * 3 * time_last_select * 3);
	f32 col = select_offset + 1.5f;
	f32 a[3], b[3], c[3];

	a[0] = 20 * move_dir;
	a[1] = -10;
	a[2] = -5.5f;

	b[0] = 0;
	b[1] = -ind;

	if (option_selected == ind)
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

static void _title_menu_option_draw(s8 move_dir,
		f32 music_t, f32 t, int ind, u32 tex)
{
	f32 a[3], b[3], c[3];

	glLoadIdentity();
	switch (music_state)
	{
	case 0:
	case 1:
		return;

	case 2:
		_title_menu_option_draw_main(t, move_dir, ind, music_t);
		break;

	case 3:
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
	smesh_draw_tex(text_mesh, tex);
}

static void _title_menu_draw(f32 music_t, f32 t)
{
	if (music_state < 2)
		return;

	_title_menu_option_draw(1, music_t, t, 0, new_game_text.id);
	_title_menu_option_draw(-1, music_t, t, 1, continue_text.id);
	_title_menu_option_draw(1, music_t, t, 2, options_text.id);
}

static void _title_music_volume(const f32 music_t)
{
	mixer_ch_set_vol(music_ch_last, 1.0f - music_t, 1.0f - music_t);
	mixer_ch_set_vol(music_ch_cur, music_t, music_t);
	if (music_ch_last == SFXC_MUSIC0 || music_ch_last == SFXC_MUSIC2)
	{
		mixer_ch_set_vol(music_ch_last, 0, 0);
		mixer_ch_set_vol(music_ch_cur, 1, 1);
	}
}

void title_draw(f32 subtick)
{
	_title_load();

	gl_context_begin();
	glClearColor(bg_is_white, bg_is_white, bg_is_white, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	f32 music_beatsf = lerpf(music_beats_last, music_beats, subtick);
	f32 alpha = 1.0f - (music_beatsf - (int)music_beatsf);

	alpha *= alpha;
	glMatrixMode(GL_MODELVIEW);

	f32 music_t = lerpf(clampf(music_state_timer_last, 0, 1),
		     clampf(music_state_timer, 0, 1), subtick);
	f32 t = lerpf(frame_counter - 1, frame_counter, subtick) / 12.0f;

	_title_logo_draw(music_t, t, subtick);
	_title_music_volume(music_t);
	if (music_state == 1)
	{
		glLoadIdentity();
		glScalef(2, 1, 1);
		glTranslatef(0, -2.5f, -2);
		f32 blink = (music_beats * 0.25f) - (int)(music_beats * 0.25f);

		blink = (int)(blink * 2) & 1;
		glColor3f(blink, blink, blink);
		smesh_draw_tex(text_mesh, press_start_text.id);
	}
	if (bg_is_white)
	{
		gl_context_end();
		return;
	}

	_title_menu_draw(music_t, t);
	glDisable(GL_BLEND);
	gl_context_end();
}
