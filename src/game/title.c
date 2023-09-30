#include <GL/gl.h>
#include <GL/gl_integration.h>

#include "engine/sfx.h"
#include "engine/config.h"
#include "engine/util.h"
#include "engine/vector.h"
#include "engine/texture.h"

#include "game/title.h"

#define MUSIC_BPM 178
#define MUSIC_DELTA ((MUSIC_BPM / 60.0f) / (float)CONF_TICKRATE)

static bool is_loaded = false;
static long frame_counter = 0;

static int music_state_last;
static int music_state;
static float music_beats_last;
static float music_beats;
static float music_state_timer_last;
static float music_state_timer;
static int music_ch_last;
static int music_ch_cur;

static int option_last_frame = 0;
static int option_selected_last = 0;
static int option_selected = 0;

static texture_t hungover_text;
static texture_t new_game_text;
static texture_t continue_text;
static texture_t options_text;
static texture_t press_start_text;

static bool bg_is_white = false;

static smesh_t *text_mesh;

void _title_load(void)
{
	if(is_loaded)
		return;

	hungover_text = texture_create_file("rom:/title_text.ia4.sprite");
	new_game_text = texture_create_file("rom:/new_game.ia4.sprite");
	continue_text = texture_create_file("rom:/continue.ia4.sprite");
	options_text = texture_create_file("rom:/options.ia4.sprite");
	press_start_text= texture_create_file("rom:/press_start.ia4.sprite");

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

	const uint16_t indis[6] = {0, 1, 2, 2, 1, 3};
	text_mesh = smesh_create_data("UI", 4, 6, verts, indis);

	is_loaded = true;
}

void _title_unload(void)
{
	if(!is_loaded)
		return;

	texture_destroy(&press_start_text);
	texture_destroy(&options_text);
	texture_destroy(&continue_text);
	texture_destroy(&new_game_text);
	texture_destroy(&hungover_text);

	is_loaded = false;
}

enum scene_index title_update(update_parms_t uparms)
{
	music_state_last = music_state;
	switch(music_state) {
	case 0:
	case 1:
		music_state += uparms.down.c->start;
		break;

	case 2:
		switch(option_selected) {
		case 0:
		case 1:
			music_state +=
				uparms.down.c->start || uparms.down.c->A;
			break;

		case 2:
			music_state +=
				uparms.down.c->start || uparms.down.c->A;
			break;
		}
		break;

	case 3:
		break;

	case 4:
		music_state -= uparms.down.c->B * 2;
		break;
	}
	

	static bool stick_option_change = false;
	option_selected_last = option_selected;
	if(music_state == 2) {
		/* Buttons */
		option_selected +=
			(uparms.down.c->C_down || uparms.down.c->down);
		option_selected -=
			(uparms.down.c->C_up || uparms.down.c->up);

		/* Joystick */
		float stick_y = (float)uparms.held.c->y / -85.0f;
		if(fabsf(stick_y) >= 0.5f) {
			if(!stick_option_change) {
				stick_option_change = true;
				if(stick_y > 0)
					option_selected++;
				else
					option_selected--;
			}
		} else {
			stick_option_change = false;
		}
	}
	if(option_selected > 2)
		option_selected = 0;
	if(option_selected < 0)
		option_selected = 2;

	if(option_selected_last != option_selected)
		option_last_frame = frame_counter;

	music_beats_last = music_beats;
	music_beats += MUSIC_DELTA;
	if(music_beats >= 31.5f && music_state == 0)
		music_state = 1;

	if(music_state_last != music_state) {
		music_ch_last = music_ch_cur;
		music_ch_cur = music_state;

		music_state_timer = 0.0f;

		if(music_ch_cur == SFXC_MUSIC3) {
			mixer_ch_stop(SFXC_MUSIC0);
			mixer_ch_stop(SFXC_MUSIC1);
			mixer_ch_stop(SFXC_MUSIC2);
			mixer_ch_stop(SFXC_MUSIC4);
			wav64_play(&title_music3, SFXC_MUSIC3);
		}

	}

	music_state_timer_last = music_state_timer;
	music_state_timer += 0.06f;

	frame_counter++;

	if(music_state == 3 && music_state_timer >= 4.0f) {
		_title_unload();
		return SCENE_TESTROOM;
	}

	return SCENE_TITLE;
}

static void _title_logo_draw(float music_t, float t, float subtick)
{
	int num_it = 5;
	for(int i = 0; i < num_it; i++) {
		glLoadIdentity();
		float a[3], b[3], c[3];
		float beats_lerp = lerpf(music_beats_last, music_beats, subtick);
		float difft = 0.0f;
		switch(music_state) {
		case 0:
			a[0] = 0.0f; a[1] = 0.0f; a[2] = -10.0f;
			b[0] = 0.0f; b[1] = 0.0f; b[2] =  0.0f;
			beats_lerp = clampf(beats_lerp - 29.5f, 0, 2) / 2.0f;
			if(beats_lerp <= 0.0f)
				return;
			vector_lerp(a, b, beats_lerp * beats_lerp, c);
			glTranslatef(c[0], c[1], c[2]);
			glRotatef(beats_lerp * 720 * beats_lerp, 0, 0, 1);
			break;
		
		case 1:
			break;

		case 2:
			a[0] = 0.0f; a[1] = 0.0f; a[2] =  0.0f;
			b[0] = 0.0f; b[1] = 1.5f; b[2] =  -1.0f;
			if(music_ch_last == 4)
				vector_copy(b, a);
			vector_smooth(a, b, music_t, c);
			glTranslatef(c[0], c[1], c[2]);
			break;

		case 3:
			static float beats_start = 0.0f;
			if(music_state_last == 2)
				beats_start = beats_lerp;
			a[0] = 0.0f; a[1] = 1.5f; a[2] = -1.0f;
			b[0] = 0.0f; b[1] = 0.0f; b[2] =  0.0f;
			difft = (beats_lerp - beats_start) / 3.5f;
			difft *= difft * difft;
			difft = clampf(difft, 0, 1);
			vector_lerp(a, b, difft, c);

			glTranslatef(c[0], c[1], c[2]);
			glRotatef(smoothf(0, 720, difft), 0, 0, 1);
			break;

		case 4:
			glTranslatef(0, 1.5f, -1.0f);
			break;
		}

		bool only_one = difft >= 1.0f;
		if(only_one) {
			glColor3f(0, 0, 0);
			glTranslatef(0, 0, 1.2f);
			bg_is_white = true;
		} else {
			float z_trans;
			if(i != num_it - 1)
				z_trans = (i * 0.4f) -
					wrapf(beats_lerp * 0.125f, 0.4f);
			else
				z_trans = 1.2f;
			float ti = smoothf(t, t + i, (1.2f - z_trans) / 1.6f);
			glTranslatef(sinf(ti) * 0.04f * (1.0f - difft), 
					sinf(ti * 1.5f) * 0.04f * (1.0f - difft),
					z_trans);
			glRotatef(sinf(ti) * 4 * (1.0f - difft), 0, 0, 1);

			float bright = (z_trans + 0.4f) / 1.6f;
			glScalef(bright, bright, 1);
			bright *= bright * bright;
			glColor3f(bright, bright, bright);
		}

		smesh_draw(text_mesh, hungover_text.id);

		if(only_one)
			return;
	}
}

static void _title_menu_option_draw(int move_dir,
		GLuint tex_id, float music_t, float t, int ind)
{
	glLoadIdentity();
	float a[3], b[3], c[3];
	switch(music_state) {
	case 0:
	case 1:
		return;

	case 2:
		float time_last_select = (float)((t * 12) - option_last_frame) /
			CONF_TICKRATE;
		float select_offset = lerpf(-1.5f, -0.5f,
				time_last_select * 3 * time_last_select * 3);
		float select_offset_bob = lerpf(-1.5f,
				-0.5f + (sinf(time_last_select * 8) / 8.0f),
				time_last_select * 3 * time_last_select * 3);
		a[0] = 20 * move_dir; a[1] = -10; a[2] = -5.5f;
		b[0] = 0; b[1] = -ind;
		float col = select_offset + 1.5f;
		if(option_selected == ind) {
			glColor3f(col, col, col);
			b[2] = select_offset_bob;
		} else {
			col = (1.0f - col) * 0.5f + 0.5f;
			glColor3f(col, col, col);
			b[2] = 1.0f / select_offset_bob;
		}
		vector_lerp(a, b, music_t, c);
		glTranslatef(c[0], c[1], c[2]);
		break;

	case 3:
		a[0] = 0; a[1] = -ind; a[2] = -1.5f;
		b[0] = 10 * move_dir; b[1] = 5; b[2] = 2.5f;
		vector_lerp(a, b, music_t, c);
		glTranslatef(c[0], c[1], c[2]);
		break;
	}

	float to = t + (8 * ind);
	glTranslatef(sinf(to) * 0.04f, sinf(to * 1.5f) * 0.04f, 0);
	glRotatef(sinf(to) * 4, 0, 0, 1);
	smesh_draw(text_mesh, tex_id);
}

static void _title_menu_draw(float music_t, float t)
{
	if(music_state < 2)
		return;

	_title_menu_option_draw(1, new_game_text.id, music_t, t, 0);
	_title_menu_option_draw(-1, continue_text.id, music_t, t, 1);
	_title_menu_option_draw(1, options_text.id, music_t, t, 2);
}

void title_draw(float subtick)
{
	_title_load();

	gl_context_begin();
	glClearColor(bg_is_white, bg_is_white, bg_is_white, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	float music_beatsf = lerpf(music_beats_last, music_beats, subtick);
	float alpha = 1.0f - (music_beatsf - (int)music_beatsf);
	alpha *= alpha;

	glMatrixMode(GL_MODELVIEW);
	float music_t = lerpf(clampf(music_state_timer_last, 0, 1),
			clampf(music_state_timer, 0, 1), subtick);
	float t = lerpf(frame_counter - 1, frame_counter, subtick) / 12.0f;
	_title_logo_draw(music_t, t, subtick);

	if(music_ch_last == SFXC_MUSIC0 || music_ch_last == SFXC_MUSIC2) {
		mixer_ch_set_vol(music_ch_last, 0, 0);
		mixer_ch_set_vol(music_ch_cur, 1, 1);
	} else  {
		mixer_ch_set_vol(music_ch_last, 1.0f - music_t, 1.0f - music_t);
		mixer_ch_set_vol(music_ch_cur, music_t, music_t);
	}

	if(music_state == 1) {
		glLoadIdentity();
		glScalef(2, 1, 1);
		glTranslatef(0, -2.5f, -2);
		float blink = (music_beats * 0.25f) - (int)(music_beats * 0.25f);
		blink = (int)(blink * 2) & 1;
		glColor3f(blink, blink, blink);
		smesh_draw(text_mesh, press_start_text.id);
	}

	if(bg_is_white) {
		gl_context_end();
		return;
	}

	_title_menu_draw(music_t, t);

	glDisable(GL_BLEND);

	gl_context_end();
}
