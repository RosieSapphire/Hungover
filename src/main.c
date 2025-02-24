#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>

#include "types.h"
#include "config.h"
#include "input.h"
#include "t3d_ext.h"
#include "util.h"

#include "engine/player.h"
#include "engine/scene.h"
#include "engine/sfx.h"
#include "engine/ui.h"

static f32 time_accumulated = 0.f;
static s32 dfs_handle;

static struct scene scene;

static T3DViewport viewport;
static u8 ambient_color[4] = { 0xFF, 0xFF, 0xFF, 0xFF };

static void _init(void);
static void _update(const f32 dt);
static f32 _update_renderer(const f32 dt);
static void _render(const f32 subtick);
static void _free(void);

int main(void)
{
	_init();

	for (;;) {
		f32 frame_time = display_get_delta_time();
		const f32 dt = 1.f / (f32)TICKRATE;

		time_accumulated += frame_time;
		while (time_accumulated >= dt) {
			_update(dt);
			time_accumulated -= dt;
		}

		const f32 subtick = _update_renderer(dt);
		_render(subtick);
		sfx_update();
	}

	_free();

	return 0;
}

static void _init(void)
{
	display_init(DISPLAY_RESOLUTION, DISPLAY_BITDEPTH, DISPLAY_NUM_BUFFERS,
		     DISPLAY_GAMMA, DISPLAY_FILTERS);
	rdpq_init();
#ifdef DEBUG_ENABLED
	debug_init_isviewer();
	debug_init_usblog();
	rdpq_debug_start();
#endif
	dfs_handle = dfs_init(DFS_DEFAULT_LOCATION);
	asset_init_compression(1);
	ui_init();
	input_init();
	sfx_init();

	viewport = t3d_viewport_create();

	t3d_init((T3DInitParams){ 0 });

	scene = scene_init_from_file("rom:/Scn.ApartmentTest.scn");
	player_init();

	time_accumulated = 0.f;
}

static void _update(const f32 dt)
{
	input_poll();

	player_update(&scene, dt);
	scene_update(&scene, dt);
}

static f32 _update_renderer(const f32 dt)
{
	f32 subtick = time_accumulated / dt;

	t3d_viewport_attach(&viewport);
	t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(VIEWPORT_FOV),
				    VIEWPORT_NEAR, VIEWPORT_FAR);
	player_to_viewport(&viewport, subtick);

	return subtick;
}

static void _render(const f32 subtick)
{
	rdpq_attach(display_get(), display_get_zbuf());

	t3d_frame_start();
	t3d_screen_clear_color(color_from_packed16(0x0001));
	t3d_screen_clear_depth();
	t3d_light_set_ambient(ambient_color);

	scene_render(&scene, subtick);
	ui_render();

	rdpq_detach_show();
}

static void _free(void)
{
	player_free();
	scene_free(&scene);

	t3d_destroy();

	ui_free();
	dfs_close(dfs_handle);
	dfs_handle = -1;

#ifdef DEBUG_ENABLED
	rdpq_debug_stop();
#endif

	sfx_free();
	input_free();
	rdpq_close();
	display_close();
}
