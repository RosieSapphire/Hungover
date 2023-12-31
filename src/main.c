#include <libdragon.h>
#include <GL/gl.h>
#include <GL/gl_integration.h>

#include "engine/config.h"
#include "engine/sfx.h"
#include "engine/util.h"
#include "engine/texture.h"
#include "engine/update.h"

#include "game/title.h"
#include "game/testroom.h"

static surface_t depth_buffer;
static u32 ticks_accum;

/**
 * _init - Initialization Function
 *
 * Description: Initializes N64, Libdragon, and other shit that needs to be
 */
static void _init(void)
{
	srand(TICKS_READ());
	display_init(CONF_RESOLUTION, CONF_BITDEPTH, CONF_NUMBUF,
			CONF_GAMMA, CONF_ANTIALIAS);
	rdpq_init();

	gl_init();
	textures_init();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	dfs_init(DFS_DEFAULT_LOCATION);
	audio_init(32000, 1);
	mixer_init(SFXC_COUNT);
	joypad_init();

	debug_init_isviewer();
	debug_init_usblog();
	/*
	 * rdpq_debug_start();
	 */

	sfx_init();

	if (scene_index == SCENE_TITLE)
	{
		wav64_play(&title_music0, SFXC_MUSIC0);
		wav64_play(&title_music1, SFXC_MUSIC1);
		wav64_play(&title_music2, SFXC_MUSIC2);
		wav64_play(&title_music4, SFXC_MUSIC4);
	}

	depth_buffer = surface_alloc(FMT_RGBA16, CONF_WIDTH, CONF_HEIGHT);
}

/**
 * _draw - Draw Function
 *
 * Description: Draws whatever scene needs to be drawn
 */
static void _draw(void)
{
	float subtick = (float)ticks_accum / (float)CONF_DELTATICKS;
	surface_t *col = display_get();

	void (*draw_funcs[SCENE_COUNT])(float) = {
		title_draw, testroom_draw,
	};

	rdpq_attach_clear(col, &depth_buffer);
	(*draw_funcs[scene_index])(subtick);
	rdpq_detach_show();
}

/**
 * _update - Update Function
 * @ticks_delta: Delta between ticks_last and ticks_now
 *
 * Description: Updates whatever scene needs to be updated
 */
static void _update(const u32 ticks_delta)
{
	enum scene_index (*update_funcs[SCENE_COUNT])(struct update_parms) = {
		title_update,
		testroom_update,
	};

	ticks_accum += ticks_delta;
	while (ticks_accum >= CONF_DELTATICKS)
	{
		joypad_poll();
		struct update_parms uparms = {
			joypad_get_buttons_pressed(JOYPAD_PORT_1),
			joypad_get_buttons_held(JOYPAD_PORT_1),
			joypad_get_inputs(JOYPAD_PORT_1),
		};

		scene_index = (*update_funcs[scene_index])(uparms);
		ticks_accum -= CONF_DELTATICKS;
	}
}

/**
 * _loop - Main Loop
 *
 * Description: Main while loop for game
 */
static void _loop(void)
{
	static u32 ticks_last;
	long ticks_now = get_ticks();
	long ticks_delta = TICKS_DISTANCE(ticks_last, ticks_now);

	ticks_last = ticks_now;
	_draw();
	_update(ticks_delta);

	if (!audio_can_write())
		return;

	mixer_poll(audio_write_begin(), audio_get_buffer_length());
	audio_write_end();
}

/**
 * main - Main Function
 *
 * Description: I shouldn't have to explain what this is
 * Return: 0 = Success, else = Failure
 */
int main(void)
{
	_init();

	while (1)
		_loop();

	return (0);
}
