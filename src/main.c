#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>

#include "t3d_ext.h"

#define INTERPOLATION 1
#define TICKRATE 24

#define TEST_CUBE_ROTSPD 90.f

struct test_cube {
        T3DModel *mdl;
        T3DMat4FP *mtx;
        rspq_block_t *dl;
        float rotdeg_old;
        float rotdeg;
};

static struct test_cube test_cube_create(void);
static void test_cube_matrix_setup(struct test_cube *tc, const float subtick);
static void test_cube_destroy(struct test_cube *tc);

static void update_loop(struct test_cube *tc, const float fixedtime);

int main(void)
{
        T3DViewport viewport;
        int dfs_handle;
        float time_accum;

        struct test_cube cube;

        T3DVec3 cam_eye, cam_foc, cam_up;
        T3DVec3 light_dir;
        uint8_t light_col[4], light_ambi[4];

        /* Initialize Libdragon. */
        display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3,
                     GAMMA_NONE, FILTERS_RESAMPLE_ANTIALIAS);
        rdpq_init();
#ifdef DEBUG
        debug_init_usblog();
        debug_init_isviewer();
        rdpq_debug_start();
#endif
        asset_init_compression(COMPRESS_LEVEL);
        dfs_handle = dfs_init(DFS_DEFAULT_LOCATION);

        /* Initialize Tiny3D. */
        {
                T3DInitParams init_params;

                memset(&init_params, 0, sizeof(init_params));
                t3d_init(init_params);
        }

        viewport = t3d_viewport_create();

        /* Initialize game. */
        cube = test_cube_create();

        cam_eye = t3d_vec3_make(0.f, 160.f, 40.f);
        cam_foc = t3d_vec3_zero();
        cam_up = t3d_vec3_zup();

        light_dir = t3d_vec3_make(-1.f, 1.f, 0.f);
        t3d_vec3_norm(&light_dir);
        light_col[0] = 0xEE;
        light_col[1] = 0xAA;
        light_col[2] = 0xAA;
        light_col[3] = 0xFF;
        light_ambi[0] = 0x50;
        light_ambi[1] = 0x50;
        light_ambi[2] = 0x64;
        light_ambi[3] = 0xFF;

        /* Main loop. */
        time_accum = 0.f;

        for (;;) {
                const float fixedtime = 1.f / TICKRATE;
                float subtick;

                /* Updating */
                for (time_accum += display_get_delta_time();
                     time_accum >= fixedtime;
                     time_accum -= fixedtime) {
                        update_loop(&cube, fixedtime);
                }

                /* Updating -> Rendering */
#if (INTERPOLATION == 1)
                subtick = time_accum / fixedtime;
#else
                subtick = 1.f;
#endif

                t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(90.f),
                                            10.f, 150.f);
                t3d_viewport_look_at(&viewport, &cam_eye, &cam_foc, &cam_up);

                test_cube_matrix_setup(&cube, subtick);

                /* Rendering */
                rdpq_attach(display_get(), display_get_zbuf());
                t3d_frame_start();
                t3d_viewport_attach(&viewport);
                t3d_screen_clear_color(RGBA32(0xFF, 0x7F, 0x64, 0xFF));
                t3d_screen_clear_depth();

                t3d_light_set_ambient(light_ambi);
                t3d_light_set_directional(0, light_col, &light_dir);
                t3d_light_set_count(1);

                rspq_block_run(cube.dl);

                rdpq_detach_show();
        }

        /* Terminate Tiny3D. */
        test_cube_destroy(&cube);
        t3d_destroy();

        /* Terminate Libdragon. */
        dfs_close(dfs_handle);
        rdpq_close();
#ifdef DEBUG
        rdpq_debug_stop();
#endif
        display_close();
}

static struct test_cube test_cube_create(void)
{
        struct test_cube tc;

        tc.mdl = t3d_model_load("rom:/cube.t3dm");
        tc.mtx = malloc_uncached(sizeof(*tc.mtx));
        tc.rotdeg_old = 0.f;
        tc.rotdeg = 0.f;

        rspq_block_begin();
        t3d_matrix_push(tc.mtx);
        t3d_model_draw(tc.mdl);
        t3d_matrix_pop(1);
        tc.dl = rspq_block_end();

        return tc;
}

static void test_cube_matrix_setup(struct test_cube *tc, const float subtick)
{
        float scl[3], rot[3], pos[3];

        scl[0] = 1.f;
        scl[1] = 1.f;
        scl[2] = 1.f;

        rot[0] = 0.f;
        rot[1] = 0.f;
        rot[2] = T3D_DEG_TO_RAD(t3d_lerpf(tc->rotdeg_old, tc->rotdeg, subtick));

        pos[0] = 0.f;
        pos[1] = 0.f;
        pos[2] = 0.f;

        t3d_mat4fp_from_srt_euler(tc->mtx, scl, rot, pos);
}

static void test_cube_destroy(struct test_cube *tc)
{
        rspq_block_free(tc->dl);
        free_uncached(tc->mtx);
        t3d_model_free(tc->mdl);
}

static void update_loop(struct test_cube *tc, const float fixedtime)
{
        tc->rotdeg_old = tc->rotdeg;
        tc->rotdeg += TEST_CUBE_ROTSPD * fixedtime;
        if (tc->rotdeg >= 360.f) {
                tc->rotdeg_old -= 360.f;
                tc->rotdeg -= 360.f;
        }
}
