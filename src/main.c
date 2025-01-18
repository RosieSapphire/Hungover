#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>

#include "config.h"
#include "util.h"

#include "engine/controller.h"
#include "engine/player.h"

#define DEBUG_TOGGLE

int main(void)
{
	display_init(DISPLAY_RESOLUTION, DISPLAY_BITDEPTH, DISPLAY_NUM_BUFFERS,
		     DISPLAY_GAMMA, DISPLAY_FILTERS);
	rdpq_init();
#ifdef DEBUG_TOGGLE
	debug_init_isviewer();
	debug_init_usblog();
	rdpq_debug_start();
#endif
	int dfs_handle = dfs_init(DFS_DEFAULT_LOCATION);
	asset_init_compression(1);
	controllers_init(CONTROLLER_FLAG_PORT1);

	T3DViewport viewport = t3d_viewport_create();
	uint8_t ambient_color[4] = { 0xFF, 0xFF, 0xFF, 0xFF };

	t3d_init((T3DInitParams){ 0 });

	player_t player;
	player = player_init();

	/* bubby */
	T3DModel *bubby_mdl = t3d_model_load("rom:/bubby-cube.t3dm");
	T3DMat4FP *bubby_matrix[6];
	for (int i = 0; i < 6; i++) {
		bubby_matrix[i] = malloc_uncached(sizeof **bubby_matrix);
	}

	rspq_block_t *bubby_dl;
	rspq_block_begin();
	for (int i = 0; i < 6; i++) {
		t3d_matrix_push(bubby_matrix[i]);
		t3d_model_draw(bubby_mdl);
		t3d_matrix_pop(1);
	}
	bubby_dl = rspq_block_end();

	float time_accumulated = 0.f;

	for (;;) {
		/**********
		 * UPDATE *
		 **********/
		float frame_time = display_get_delta_time();
		time_accumulated += frame_time;
		while (time_accumulated >= DELTA_TIME) {
			controllers_update();
			player_update(&player);
			time_accumulated -= DELTA_TIME;
		}

		/********************
		 * UPDATE -> RENDER *
		 ********************/
		float subtick = time_accumulated / DELTA_TIME;

		t3d_viewport_attach(&viewport);
		t3d_viewport_set_projection(&viewport,
					    T3D_DEG_TO_RAD(VIEWPORT_FOV),
					    VIEWPORT_NEAR, VIEWPORT_FAR);
		camera_to_viewport(&viewport, &player.cam, subtick);

		{ /* bubby matrix */
			float scale[3] = { 1.f, 1.f, 1.f };
			float rotation[3] = { 0.f, 0.f, 0.f };
			float positions[6][3] = {
				{ 0.f, 0.f, -128.f }, { 0.f, 0.f, 128.f },
				{ 128.f, 0.f, 0.f },  { -128.f, 0.f, 0.f },
				{ 0.f, 128.f, 0.f },  { 0.f, -128.f, 0.f }
			};
			for (int i = 0; i < 6; i++) {
				t3d_mat4fp_from_srt_euler(bubby_matrix[i],
							  scale, rotation,
							  positions[i]);
			}
		}

		/**********
		 * RENDER *
		 **********/
		rdpq_attach(display_get(), display_get_zbuf());
		t3d_frame_start();
		t3d_screen_clear_color(RGBA16(0x3, 0x6, 0x9, 0x1F));
		t3d_screen_clear_depth();
		t3d_light_set_ambient(ambient_color);
		rspq_block_run(bubby_dl);
		rdpq_detach_show();
	}

	for (int i = 0; i < 4; i++) {
		free_uncached(bubby_matrix[i]);
	}
	t3d_model_free(bubby_mdl);
	t3d_destroy();

	dfs_close(dfs_handle);
#ifdef DEBUG_TOGGLE
	rdpq_debug_stop();
#endif
	controllers_terminate();
	rdpq_close();
	display_close();

	return 0;
}
