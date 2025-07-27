#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>

#include "util.h"

#include "engine/object.h"

#define INTERPOLATION 1
#define TICKRATE 24

#define VIEWPORT_NEAR ( .1f * MODEL_SCALE)
#define VIEWPORT_FAR (1.5f * MODEL_SCALE)
#define VIEWPORT_FOV_DEG 90.f

#define CUBE_ROTATION_SPEED 1.25f

enum {
        TEST_OBJECT_CUBE_INDEX,
        TEST_OBJECT_FLOOR_INDEX,
        TEST_OBJECT_COUNT
};

static void test_objects_create(struct object *objects);
static void test_object_cube_update(struct object *cube,
                                    const float fixed_time);
static void test_object_floor_update(struct object *floor,
                                     const float fixed_time);
static void test_objects_run_updates(struct object *objects,
                                     const float fixed_time);
static void test_objects_setup_matrices(struct object *objects,
                                        const float subtick);
static void test_objects_render(struct object *objects);
static void test_objects_destroy(struct object *objects);

static void (*test_object_update_functions[TEST_OBJECT_COUNT])
            (struct object *, const float) = {
        test_object_cube_update,
        test_object_floor_update
};

int main(void)
{
        T3DViewport viewport;
        int dfs_handle;
        float time_accumulated;

        struct object test_objects[TEST_OBJECT_COUNT];

        T3DVec3 camera_eye, camera_focus, camera_up;
        T3DVec3 light_direction;
        uint8_t light_col_direction[4], light_col_ambi[4];

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
                T3DInitParams initparams;

                memset(&initparams, 0, sizeof(initparams));
                t3d_init(initparams);
        }

        viewport = t3d_viewport_create();

        /* Initialize game. */
        test_objects_create(test_objects);

        camera_eye = t3d_vec3_make(0.f, 1.25f, 1.25f);
        camera_focus = t3d_vec3_make(0.f, 0.f, .5f);
        camera_up = t3d_vec3_zup();

        light_direction = t3d_vec3_make(-1.f, 1.f, 0.f);
        t3d_vec3_norm(&light_direction);
        light_col_direction[0] = 0xEE;
        light_col_direction[1] = 0xAA;
        light_col_direction[2] = 0xAA;
        light_col_direction[3] = 0xFF;
        light_col_ambi[0] = 0x19;
        light_col_ambi[1] = 0x32;
        light_col_ambi[2] = 0x4B;
        light_col_ambi[3] = 0xFF;

        /* Main loop. */
        time_accumulated = 0.f;

        for (;;) {
                const float fixed_time = 1.f / TICKRATE;
                float subtick;

                /* Updating */
                for (time_accumulated += display_get_delta_time();
                     time_accumulated >= fixed_time;
                     time_accumulated -= fixed_time) {
                        test_objects_run_updates(test_objects, fixed_time);
                }

                /* Updating -> Rendering */
#if (INTERPOLATION == 1)
                subtick = time_accumulated / fixed_time;
#else
                subtick = 1.f;
#endif

                t3d_viewport_set_projection(&viewport,
                                            T3D_DEG_TO_RAD(VIEWPORT_FOV_DEG),
                                            VIEWPORT_NEAR, VIEWPORT_FAR);
                {
                        T3DVec3 camera_eye_scaled, camera_focus_scaled;

                        camera_eye_scaled = t3d_vec3_scale(&camera_eye,
                                                           MODEL_SCALE);
                        camera_focus_scaled = t3d_vec3_scale(&camera_focus,
                                                             MODEL_SCALE);
                        t3d_viewport_look_at(&viewport, &camera_eye_scaled,
                                             &camera_focus_scaled, &camera_up);
                }

                test_objects_setup_matrices(test_objects, subtick);

                /* Rendering */
                rdpq_attach(display_get(), display_get_zbuf());
                t3d_frame_start();
                rdpq_mode_dithering(DITHER_NOISE_NONE);

                t3d_viewport_attach(&viewport);
                {
                        uint32_t ambi32;

                        ambi32 = U8ARR_TO_U32PACK(light_col_ambi);
                        t3d_screen_clear_color(color_from_packed32(ambi32));
                }
                t3d_screen_clear_depth();

                t3d_light_set_ambient(light_col_ambi);
                t3d_light_set_directional(0, light_col_direction,
                                          &light_direction);
                t3d_light_set_count(1);

                test_objects_render(test_objects);

                rdpq_detach_show();
        }

        /* Terminate Tiny3D. */
        test_objects_destroy(test_objects);
        t3d_destroy();

        /* Terminate Libdragon. */
        dfs_close(dfs_handle);
        rdpq_close();
#ifdef DEBUG
        rdpq_debug_stop();
#endif
        display_close();
}

static void test_objects_create(struct object *objects)
{
        const char *paths[TEST_OBJECT_COUNT] = {
                "rom:/cube.t3dm",
                "rom:/floor.t3dm"
        };

        T3DVec3 positions[TEST_OBJECT_COUNT],
                rotations[TEST_OBJECT_COUNT],
                scales[TEST_OBJECT_COUNT];
        int i;

        for (i = 0; i < TEST_OBJECT_COUNT; ++i) {
                positions[i] = t3d_vec3_zero();
                rotations[i] = t3d_vec3_zero();
                scales[i] = t3d_vec3_one();
                objects[i] = object_create(paths[i], positions + i,
                                           rotations + i, scales + i,
                                           test_object_update_functions[i]);
        }
}

static void test_object_cube_update(struct object *cube,
                                    const float fixed_time)
{
        cube->rotation_euler_a = cube->rotation_euler_b;

        cube->rotation_euler_b.v[2] += CUBE_ROTATION_SPEED * fixed_time;
        if (cube->rotation_euler_b.v[2] >= 2.f * T3D_PI) {
                cube->rotation_euler_a.v[2] -= 2.f * T3D_PI;
                cube->rotation_euler_b.v[2] -= 2.f * T3D_PI;
        }
}

static void test_object_floor_update(struct object *floor,
                                     const float fixed_time)
{
        static float t = 0.f;

        floor->position_a = floor->position_b;

        t += fixed_time * T3D_PI * 2.f;
        floor->position_b.v[0] = cosf(t) * .25f;
        floor->position_b.v[1] = sinf(t) * .25f;
        /*
        radian_wrap_2pi_dual(floor->position_a.v + 0, floor->position_b.v + 0);
        radian_wrap_2pi_dual(floor->position_a.v + 1, floor->position_b.v + 1);
        */
}

static void test_objects_run_updates(struct object *objects,
                                     const float fixed_time)
{
        int i;

        for (i = 0; i < TEST_OBJECT_COUNT; ++i) {
                struct object *object;

                object = objects + i;
                if (object->update_function)
                        object->update_function(object, fixed_time);
        }
}

static void test_objects_setup_matrices(struct object *objects,
                                        const float subtick)
{
        int i;

        for (i = 0; i < TEST_OBJECT_COUNT; ++i)
                object_setup_matrix(objects + i, subtick);
}

static void test_objects_render(struct object *objects)
{
        int i;

        for (i = 0; i < TEST_OBJECT_COUNT; ++i)
                rspq_block_run(objects[i].displaylist);
}

static void test_objects_destroy(struct object *objects)
{
        int i;

        for (i = 0; i < TEST_OBJECT_COUNT; ++i)
                object_destroy(objects + i);
}
