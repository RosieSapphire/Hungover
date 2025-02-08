#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>

#include "config.h"
#include "util.h"
#include "input.h"
#include "t3d_ext.h"

#include "engine/scene.h"
#include "engine/player.h"
#include "engine/ui.h"

/* general */
static float time_accumulated = 0.f;

/* game */
static player_t player;

/* libdragon */
static int dfs_handle;

/* tiny3D */
static T3DViewport viewport;
static uint8_t ambient_color[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
/*
static uint8_t ambient_color[4] = { 0x10, 0x10, 0x10, 0xFF };
static uint8_t light_color[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
static T3DVec3 light_dir = { { 0.577f, 0.577f, 0.577f } };
*/

static scene_t scene;

/* function prototypes */
static void _init(void);
static void _update(const float dt);
static float _update_renderer(const float dt);
static void _render(const float subtick);
static void _terminate(void);

int main(void)
{
	_init();

	for (;;) {
		float frame_time = display_get_delta_time();
		const float dt = 1.f / (float)TICKRATE;

		time_accumulated += frame_time;
		while (time_accumulated >= dt) {
			_update(dt);
			time_accumulated -= dt;
		}

		const float subtick = _update_renderer(dt);
		_render(subtick);
	}

	_terminate();

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

	viewport = t3d_viewport_create();

	t3d_init((T3DInitParams){ 0 });

	scene = scene_init_from_file("rom:/Scn.ApartmentTest.scn");
	player = player_init(&scene);

	time_accumulated = 0.f;
}

static void _update(const float dt)
{
	input_poll();

	player_update(&player, &scene, dt);

	T3DVec3 player_eye, player_focus, player_dir;
	player_get_look_values(&player_eye, &player_focus, &player, 1.f);
	t3d_vec3_diff(&player_dir, &player_focus, &player_eye);
	scene_update(&scene, &player.pos, &player_dir, dt);
}

static float _update_renderer(const float dt)
{
	float subtick = time_accumulated / dt;

	t3d_viewport_attach(&viewport);
	t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(VIEWPORT_FOV),
				    VIEWPORT_NEAR, VIEWPORT_FAR);
	player_to_viewport(&viewport, &player, subtick);

	return subtick;
}

static void _render(const float subtick)
{
	rdpq_attach(display_get(), display_get_zbuf());

	t3d_frame_start();
	t3d_screen_clear_color(color_from_packed16(0x0001));
	t3d_screen_clear_depth();

	t3d_light_set_ambient(ambient_color);
	/*
	t3d_light_set_count(1);
	t3d_light_set_directional(0, light_color, &light_dir);
	*/

	scene_render(&scene, subtick);

	ui_render();

	rdpq_detach_show();
}

static void _terminate(void)
{
	player_terminate(&player);
	scene_terminate(&scene);

	t3d_destroy();

	ui_terminate();
	dfs_close(dfs_handle);
	dfs_handle = -1;

#ifdef DEBUG_ENABLED
	rdpq_debug_stop();
#endif

	input_terminate();
	rdpq_close();
	display_close();
}
