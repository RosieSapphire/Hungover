#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>

#include "config.h"
#include "util.h"
#include "input.h"
#include "t3d_ext.h"

#include "engine/object.h"
#include "engine/player.h"

/* general */
static float time_accumulated = 0.f;

/* game */
static player_t player;

/* libdragon */
static int dfs_handle;

/* tiny3D */
static T3DViewport viewport;
static uint8_t ambient_color[4] = { 0xFF, 0xFF, 0xFF, 0xFF };

static T3DModel *bubby_mdl;
static object_t bubby_objs[6];
static rspq_block_t *bubby_objs_dl;

/* function prototypes */
static void _init(void);
static void _update(const float dt);
static void _update_renderer(const float dt);
static void _render(void);
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

		_update_renderer(dt);
		_render();
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
	input_init();

	viewport = t3d_viewport_create();
	for (int i = 0; i < 4; i++) {
		ambient_color[i] = 0xFF;
	}

	t3d_init((T3DInitParams){ 0 });

	player = player_init();

	/* bubby */
	const T3DVec3 bubby_pos[6] = {
		{ { 0.f, 0.f, -128.f } }, { { 0.f, 0.f, 128.f } },
		{ { 128.f, 0.f, 0.f } },  { { -128.f, 0.f, 0.f } },
		{ { 0.f, 128.f, 0.f } },  { { 0.f, -128.f, 0.f } }
	};

	bubby_mdl = t3d_model_load("rom:/bubby-cube.t3dm");
	for (int i = 0; i < 6; i++) {
		bubby_objs[i] = object_init_from_model_pointer(bubby_mdl,
							       bubby_pos + i,
							       &T3D_VEC3_ZERO,
							       &T3D_VEC3_ONE);
	}

	bubby_objs_dl = objects_instanced_gen_dl(6, bubby_objs, bubby_mdl);

	time_accumulated = 0.f;
}

static void _update(const float dt)
{
	input_poll();

	player_update(&player, dt);

	for (int i = 0; i < 6; i++) {
		object_t *o = bubby_objs + i;

		o->rotation_old = o->rotation;
		o->rotation.v[i % 3] += dt * T3D_PI;
	}
}

static void _update_renderer(const float dt)
{
	float subtick = time_accumulated / dt;

	t3d_viewport_attach(&viewport);
	t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(VIEWPORT_FOV),
				    VIEWPORT_NEAR, VIEWPORT_FAR);
	camera_to_viewport(&viewport, &player.cam, subtick);

	for (int i = 0; i < 6; i++) {
		object_matrix_setup(bubby_objs + i, subtick);
	}
}

static void _render(void)
{
	rdpq_attach(display_get(), display_get_zbuf());

	t3d_frame_start();
	t3d_screen_clear_color(RGBA16(0x3, 0x6, 0x9, 0x1F));
	t3d_screen_clear_depth();

	t3d_light_set_ambient(ambient_color);

	rspq_block_run(bubby_objs_dl);

	rdpq_detach_show();
}

static void _terminate(void)
{
	player_terminate(&player);

	rspq_block_free(bubby_objs_dl);
	bubby_objs_dl = NULL;

	for (int i = 0; i < 6; i++) {
		object_terminate(bubby_objs + i, false);
	}

	t3d_model_free(bubby_mdl);
	bubby_mdl = NULL;

	t3d_destroy();

	dfs_close(dfs_handle);
	dfs_handle = -1;

#ifdef DEBUG_ENABLED
	rdpq_debug_stop();
#endif

	input_terminate();
	rdpq_close();
	display_close();
}
