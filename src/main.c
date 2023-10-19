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

int main(void)
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
	rdpq_debug_start();
	*/

	sfx_init();

	if(scene_index == SCENE_TITLE) {
		wav64_play(&title_music0, SFXC_MUSIC0);
		wav64_play(&title_music1, SFXC_MUSIC1);
		wav64_play(&title_music2, SFXC_MUSIC2);
		wav64_play(&title_music4, SFXC_MUSIC4);
	}

	surface_t dep = surface_alloc(FMT_RGBA16, CONF_WIDTH, CONF_HEIGHT);

	long ticks_last = get_ticks();
	long ticks_accum = 0;
	while(1) {
		long ticks_now = get_ticks();
		long ticks_delta = TICKS_DISTANCE(ticks_last, ticks_now);
		ticks_last = ticks_now;

		enum scene_index (*update_funcs[SCENE_COUNT])(update_parms_t) = {
			title_update, testroom_update,
		};

		void (*draw_funcs[SCENE_COUNT])(float) = {
			title_draw, testroom_draw,
		};

		ticks_accum += ticks_delta;
		while(ticks_accum >= CONF_DELTATICKS) {
			joypad_poll();
			update_parms_t uparms = {
				get_keys_down(),
				get_keys_held(),
			};

			scene_index = (*update_funcs[scene_index])(uparms);
			ticks_accum -= CONF_DELTATICKS;
		}

		float subtick = (float)ticks_accum / (float)CONF_DELTATICKS;
		surface_t *col = display_get();
		rdpq_attach_clear(col, &dep);
		(*draw_funcs[scene_index])(subtick);
		rdpq_detach_show();

		if(!audio_can_write())
			continue;

		short *audio_buf = audio_write_begin();
		mixer_poll(audio_buf, audio_get_buffer_length());
		audio_write_end();
	}

	return 0;
}
