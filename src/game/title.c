#include <GL/gl.h>
#include <GL/gl_integration.h>

#include "engine/sfx.h"
#include "engine/config.h"
#include "engine/util.h"
#include "engine/vector.h"
#include "engine/texture.h"

#include "game/title.h"

static bool is_loaded;
static u32 frame_counter;
static s8 option_selected;

static texture_t new_game_text;
static texture_t continue_text;
static texture_t options_text;

static u8 music_state_last;
static u8 music_state;
static u8 music_ch_last;
static u8 music_ch_cur;

static f32 music_beats_last;
static f32 music_beats;
static f32 music_state_timer_last;
static f32 music_state_timer;

static texture_t hungover_text;
static texture_t press_start_text;

static u8 bg_is_white;

static smesh_t *text_mesh;

/**
 * title_load - Loads Title Assets
 *
 * Description: Loads assets for the title screen
 */
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
	new_game_text = tex_objs_loaded[1];
	continue_text = tex_objs_loaded[2];
	options_text  = tex_objs_loaded[3];
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

/**
 * title_unload - Unloads Title Assets
 *
 * Description: Unloads assets for the title screen
 */
void title_unload(void)
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

/**
 * title_update - Updates Title Screen
 * @uparms: Input Parameters
 *
 * Description: Updates title screen functionality
 * Return: Desired Scene Index
 */
enum scene_index title_update(struct update_parms uparms)
{
	frame_counter++;
	title_music_change(uparms, &music_state_last,
		    &music_state, option_selected);
	title_music_update(&music_beats_last, &music_beats,
		    &music_state_last, &music_state,
		    &music_ch_last, &music_ch_cur,
		    &music_state_timer_last, &music_state_timer);

	if (music_state == 3 && music_state_timer >= 4.0f)
	{
		title_unload();
		return (SCENE_TESTROOM);
	}

	title_menu_option_change(uparms, &option_selected,
			  music_state, frame_counter);

	return (SCENE_TITLE);
}

/**
 * title_logo_draw - Draws Title Screen Logo
 * @mesh: Mesh quad for drawing title screen 2D assets with
 * @tid: OpenGL Texture ID
 * @music_t: Music Interpolation
 * @t: Interpolation
 * @subtick: 0-1 Value between last and current frame
 * @music_beats_last: Music Beats value last
 * @music_beats: Music Beats value current
 * @music_state_last: Music State value last
 * @music_state: Music State value current
 * @music_ch_last: Music Channel last
 * @bg_is_white: Boolean Pointer for the background being white
 *
 * Description: Draws all the iterations of the title screen logo
 */
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
		u8 do_draw = 0;

		switch (music_state)
		{
		case 0:
			do_draw = title_logo_transform_intro(&beats_lerp);
			break;
		case 1:
			do_draw = 1;
			break;
		case 2:
			do_draw = title_logo_transform_main(music_t,
				music_ch_last);
			break;
		case 3:
			do_draw = title_logo_transform_start(beats_lerp,
					   &difft, music_state_last);
			break;
		case 4:
			do_draw = 1;
			glTranslatef(0, 1.5f, -1.0f);
			break;
		}

		if (!do_draw)
			break;

		title_logo_draw_object(difft, i, num_it, beats_lerp, t,
			 mesh, tid, bg_is_white);

		if (difft >= 1.0f)
			return;
	}
}

/**
 * title_draw - Draws Title Screen
 * @subtick: Value for interpolating between last and current frame
 *
 * Description: Draws everything necessary for the title screen
 */
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
	title_logo_draw_press_start(text_mesh, press_start_text.id,
			     music_state, music_beats);
	if (bg_is_white)
	{
		gl_context_end();
		return;
	}
	title_music_volume(music_t, music_ch_last, music_ch_cur);
	if (music_state >= 2)
	{
		title_menu_option_draw(text_mesh, 1, music_state,
			 option_selected, music_t, t, 0, new_game_text.id);
		title_menu_option_draw(text_mesh, -1, music_state,
			 option_selected, music_t, t, 1, continue_text.id);
		title_menu_option_draw(text_mesh, 1, music_state,
			 option_selected, music_t, t, 2, options_text.id);
	}
	glDisable(GL_BLEND);
	gl_context_end();
}
