#include <GL/gl.h>
#include <GL/gl_integration.h>

#include "engine/sfx.h"
#include "engine/config.h"
#include "engine/util.h"
#include "engine/vector.h"
#include "engine/texture.h"

#include "game/title.h"

#define MUSIC_BPM 178
#define MUSIC_DELTA ((MUSIC_BPM / 60.0f) / (f32)CONF_TICKRATE)

static bool is_loaded;
static u32 frame_counter;
static s8 option_selected;

static int music_state_last;
static int music_state;
static f32 music_beats_last;
static f32 music_beats;
static f32 music_state_timer_last;
static f32 music_state_timer;
static int music_ch_last;
static int music_ch_cur;

static texture_t hungover_text;
static texture_t press_start_text;

static u8 bg_is_white;

static smesh_t *text_mesh;

void title_load(void)
{
	if (is_loaded)
		return;

	texture_create_file("rom:/title_text.ia8.sprite");
	texture_create_file("rom:/new_game.ia8.sprite");
	texture_create_file("rom:/continue.ia8.sprite");
	texture_create_file("rom:/options.ia8.sprite");
	texture_create_file("rom:/press_start.ia8.sprite");

	hungover_text    = tex_objs_loaded[0];
	title_menu_load();
	press_start_text = tex_objs_loaded[4];

	frame_counter = 0;
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

void title_unload(void)
{
	if (!is_loaded)
		return;

	texture_destroy(&press_start_text);
	title_menu_unload();
	texture_destroy(&hungover_text);

	is_loaded = false;
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
		title_unload();
		return (SCENE_TESTROOM);
	}

	title_menu_option_change(uparms, &option_selected,
			  music_state, frame_counter);

	return (SCENE_TITLE);
}

void title_logo_draw(const smesh_t *mesh, const u32 tid, f32 music_t, f32 t,
		     f32 subtick, f32 music_beats_last, f32 music_beats,
		     u8 music_state_last, u8 music_state,
		     const u8 music_ch_last, u8 *bg_is_white)
{
	int num_it = 5;

	for (int i = 0; i < num_it; i++)
	{
		glLoadIdentity();
		f32 beats_lerp = lerpf(music_beats_last, music_beats, subtick);
		f32 difft = 0.0f;

		if(!title_logo_transform(music_state, music_state_last,
			music_ch_last, &beats_lerp, &difft, music_t))
			break;

		title_logo_draw_object(difft, i, num_it, beats_lerp, t,
			 mesh, tid, bg_is_white);

		if (difft >= 1.0f)
			return;
	}
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
	title_load();
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

	title_logo_draw(text_mesh, hungover_text.id, music_t, t, subtick,
		 music_beats_last, music_beats, music_state_last, music_state,
		 music_ch_last, &bg_is_white);
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
	title_menu_draw(text_mesh, music_t, t, music_state, option_selected);
	glDisable(GL_BLEND);
	gl_context_end();
}
