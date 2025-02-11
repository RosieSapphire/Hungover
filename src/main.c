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
static float timeAccum = 0.f;

/* game */
static Player player;
static Scene scene;

/* libdragon */
static int dfsHandle;

/* tiny3D */
static T3DViewport viewport;
static uint8_t ambientColor[4] = { 0xFF, 0xFF, 0xFF, 0xFF };

/* function prototypes */
static void _init(void);
static void _update(const float dt);
static float _updateRenderer(const float dt);
static void _render(const float subtick);
static void _free(void);

int main(void)
{
	_init();

	for (;;) {
		float frameTime = display_get_delta_time();
		const float dt = 1.f / (float)TICKRATE;

		timeAccum += frameTime;
		while (timeAccum >= dt) {
			_update(dt);
			timeAccum -= dt;
		}

		const float subtick = _updateRenderer(dt);
		_render(subtick);
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
	dfsHandle = dfs_init(DFS_DEFAULT_LOCATION);
	asset_init_compression(1);
	uiInit();
	inputInit();

	viewport = t3d_viewport_create();

	t3d_init((T3DInitParams){ 0 });

	scene = sceneInitFromFile("rom:/Scn.ApartmentTest.scn");
	player = playerInit();

	timeAccum = 0.f;
}

static void _update(const float dt)
{
	inputPoll();

	playerUpdate(&player, &scene, dt);

	T3DVec3 playerEye, playerFocus, playerDir;
	playerGetLookValues(&playerEye, &playerFocus, &player, 1.f);
	t3d_vec3_diff(&playerDir, &playerFocus, &playerEye);
	sceneUpdate(&scene, &player.pos, &playerDir, dt);
}

static float _updateRenderer(const float dt)
{
	float subtick = timeAccum / dt;

	t3d_viewport_attach(&viewport);
	t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(VIEWPORT_FOV),
				    VIEWPORT_NEAR, VIEWPORT_FAR);
	playerToViewport(&viewport, &player, subtick);

	return subtick;
}

static void _render(const float subtick)
{
	rdpq_attach(display_get(), display_get_zbuf());

	t3d_frame_start();
	t3d_screen_clear_color(color_from_packed16(0x0001));
	t3d_screen_clear_depth();

	t3d_light_set_ambient(ambientColor);

	sceneRender(&scene, subtick);
	uiRender();

	rdpq_detach_show();
}

static void _free(void)
{
	playerFree(&player);
	sceneFree(&scene);

	t3d_destroy();

	uiFree();
	dfs_close(dfsHandle);
	dfsHandle = -1;

#ifdef DEBUG_ENABLED
	rdpq_debug_stop();
#endif

	inputFree();
	rdpq_close();
	display_close();
}
