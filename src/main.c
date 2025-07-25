#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>

#include "util.h"

#define INTERPOLATION 1
#define TICKRATE 24

#define VIEWPORT_NEAR ( .1f * MDL_SCALE)
#define VIEWPORT_FAR (1.5f * MDL_SCALE)
#define VIEWPORT_FOV_DEG 90.f

#define CUBE_ROTSPD 1.25f

enum {
        OBJ_CUBE_IND,
        OBJ_FLOOR_IND,
        OBJ_CNT
};

struct object {
        T3DModel *mdl;
        T3DMat4FP *mtx;
        rspq_block_t *dl;
        T3DVec3 pos_old;
        T3DVec3 pos;
        T3DVec3 roteul_old;
        T3DVec3 roteul;
        T3DVec3 scale_old;
        T3DVec3 scale;
        void (*update_func)(struct object *, const float fixedtime);
};

static struct object object_create(const char *mdl_path,
                                   const T3DVec3 *pos,
                                   const T3DVec3 *roteul,
                                   const T3DVec3 *scale,
                                   void (*update_func)(struct object *,
                                                       const float));
static void objects_create(struct object *objs);
static void object_setup_matrix(struct object *obj, const float subtick);
static void objects_setup_matrices(struct object *objs, const int obj_cnt,
                                   const float subtick);
static void objects_render(struct object *objs, const int obj_cnt);
static void object_destroy(struct object *obj);
static void objects_destroy(struct object *objs);

static void obj_cube_update(struct object *tc, const float fixedtime);
static void obj_floor_update(struct object *fl, const float fixedtime);

static void update_loop(struct object *objs, const int obj_cnt,
                        const float fixedtime);

static void (*obj_update_funcs[OBJ_CNT])(struct object *, const float) = {
        obj_cube_update,
        obj_floor_update
};

int main(void)
{
        T3DViewport viewport;
        int dfs_handle;
        float time_accum;

        struct object objs[OBJ_CNT];

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
        objects_create(objs);

        cam_eye = t3d_vec3_make(0.f, 1.25f, 1.25f);
        cam_foc = t3d_vec3_make(0.f, 0.f, .5f);
        cam_up = t3d_vec3_zup();

        light_dir = t3d_vec3_make(-1.f, 1.f, 0.f);
        t3d_vec3_norm(&light_dir);
        light_col[0] = 0xEE;
        light_col[1] = 0xAA;
        light_col[2] = 0xAA;
        light_col[3] = 0xFF;
        light_ambi[0] = 0x19;
        light_ambi[1] = 0x32;
        light_ambi[2] = 0x4B;
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
                        update_loop(objs, OBJ_CNT, fixedtime);
                }

                /* Updating -> Rendering */
#if (INTERPOLATION == 1)
                subtick = time_accum / fixedtime;
#else
                subtick = 1.f;
#endif

                t3d_viewport_set_projection(&viewport,
                                            T3D_DEG_TO_RAD(VIEWPORT_FOV_DEG),
                                            VIEWPORT_NEAR, VIEWPORT_FAR);
                {
                        T3DVec3 cam_eye_scaled, cam_foc_scaled;

                        cam_eye_scaled = t3d_vec3_scale(&cam_eye, MDL_SCALE);
                        cam_foc_scaled = t3d_vec3_scale(&cam_foc, MDL_SCALE);
                        t3d_viewport_look_at(&viewport, &cam_eye_scaled,
                                             &cam_foc_scaled, &cam_up);
                }

                objects_setup_matrices(objs, OBJ_CNT, subtick);

                /* Rendering */
                rdpq_attach(display_get(), display_get_zbuf());
                t3d_frame_start();
                rdpq_mode_dithering(DITHER_NOISE_NONE);

                t3d_viewport_attach(&viewport);
                t3d_screen_clear_color(
                        color_from_packed32(U8_ARR_TO_U32_PACK(light_ambi)));
                t3d_screen_clear_depth();

                t3d_light_set_ambient(light_ambi);
                t3d_light_set_directional(0, light_col, &light_dir);
                t3d_light_set_count(1);

                objects_render(objs, OBJ_CNT);

                rdpq_detach_show();
        }

        /* Terminate Tiny3D. */
        objects_destroy(objs);
        t3d_destroy();

        /* Terminate Libdragon. */
        dfs_close(dfs_handle);
        rdpq_close();
#ifdef DEBUG
        rdpq_debug_stop();
#endif
        display_close();
}

static struct object object_create(const char *mdl_path,
                                   const T3DVec3 *pos,
                                   const T3DVec3 *roteul,
                                   const T3DVec3 *scale,
                                   void (*update_func)(struct object *,
                                                       const float))
{
        struct object obj;

        obj.mdl = t3d_model_load(mdl_path);
        obj.mtx = malloc_uncached(sizeof(*obj.mtx));

        rspq_block_begin();
        t3d_matrix_push(obj.mtx);
        t3d_model_draw(obj.mdl);
        t3d_matrix_pop(1);
        obj.dl = rspq_block_end();

        obj.pos = *pos;
        obj.pos_old = obj.pos;
        obj.roteul = *roteul;
        obj.roteul_old = obj.roteul;
        obj.scale = *scale;
        obj.scale_old = obj.scale;
        obj.update_func = update_func;

        return obj;
}

static void objects_create(struct object *objs)
{
        const char *paths[OBJ_CNT] = {
                "rom:/cube.t3dm",
                "rom:/floor.t3dm"
        };

        T3DVec3 pos[OBJ_CNT], roteul[OBJ_CNT], scale[OBJ_CNT];
        int i;

        for (i = 0; i < OBJ_CNT; ++i) {
                pos[i] = t3d_vec3_zero();
                roteul[i] = t3d_vec3_zero();
                scale[i] = t3d_vec3_one();
                objs[i] = object_create(paths[i], pos + i,
                                        roteul + i, scale + i,
                                        obj_update_funcs[i]);
        }
}

static void object_setup_matrix(struct object *obj, const float subtick)
{
        T3DVec3 scale, roteul, pos, pos_orig_old, pos_orig;

        pos_orig_old = t3d_vec3_scale(&obj->pos_old, MDL_SCALE);
        pos_orig = t3d_vec3_scale(&obj->pos, MDL_SCALE);
        t3d_vec3_lerp(&pos, &pos_orig_old, &pos_orig, subtick);
        t3d_vec3_lerp(&roteul, &obj->roteul_old, &obj->roteul, subtick);
        t3d_vec3_lerp(&scale, &obj->scale_old, &obj->scale, subtick);
        t3d_mat4fp_from_srt_euler(obj->mtx, scale.v, roteul.v, pos.v);
}

static void objects_setup_matrices(struct object *objs, const int obj_cnt,
                                   const float subtick)
{
        int i;

        for (i = 0; i < obj_cnt; ++i)
                object_setup_matrix(objs + i, subtick);
}

static void objects_render(struct object *objs, const int obj_cnt)
{
        int i;

        for (i = 0; i < obj_cnt; ++i)
                rspq_block_run(objs[i].dl);
}

static void object_destroy(struct object *obj)
{
        rspq_block_free(obj->dl);
        free_uncached(obj->mtx);
        t3d_model_free(obj->mdl);
}

static void objects_destroy(struct object *objs)
{
        int i;

        for (i = 0; i < OBJ_CNT; ++i)
                object_destroy(objs + i);
}

static void obj_cube_update(struct object *tc, const float fixedtime)
{
        tc->roteul_old = tc->roteul;

        tc->roteul.v[2] += CUBE_ROTSPD * fixedtime;
        if (tc->roteul.v[2] >= 2.f * T3D_PI) {
                tc->roteul_old.v[2] -= 2.f * T3D_PI;
                tc->roteul.v[2] -= 2.f * T3D_PI;
        }
}

static void obj_floor_update(struct object *fl, const float fixedtime)
{
        static float t = 0.f;

        fl->pos_old = fl->pos;

        t += fixedtime * T3D_PI * 2.f;
        fl->pos.v[2] = -fabsf(sinf(t)) * .5f;
        if (fl->pos.v[2] >= 2.f * T3D_PI) {
                fl->pos_old.v[2] -= 2.f * T3D_PI;
                fl->pos.v[2] -= 2.f * T3D_PI;
        }
}

static void update_loop(struct object *objs, const int obj_cnt,
                        const float fixedtime)
{
        int i;

        for (i = 0; i < obj_cnt; ++i) {
                struct object *obj;

                obj = objs + i;
                if (obj->update_func)
                        obj->update_func(objs + i, fixedtime);
        }
}
