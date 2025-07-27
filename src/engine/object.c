#include "engine/object.h"

#include "util.h"

struct object object_create(const char *mdl_path, const T3DVec3 *pos,
                            const T3DVec3 *rot_eul, const T3DVec3 *scl,
                            void (*update_func)(struct object *, const float))
{
        struct object object;

        object.model = t3d_model_load(mdl_path);
        object.matrix = malloc_uncached(sizeof(*object.matrix));

        rspq_block_begin();
        t3d_matrix_push(object.matrix);
        t3d_model_draw(object.model);
        t3d_matrix_pop(1);
        object.displaylist = rspq_block_end();

        object.position_a = *pos;
        object.position_b = object.position_a;
        object.rotation_euler_a = *rot_eul;
        object.rotation_euler_b = object.rotation_euler_a;
        object.scale_a = *scl;
        object.scale_b = object.scale_a;
        object.update_function = update_func;

        return object;
}

void object_setup_matrix(struct object *obj, const float subtick)
{
        T3DVec3 scale, roteul, pos, pos_orig_old, pos_orig;

        pos_orig_old = t3d_vec3_scale(&obj->position_a, MODEL_SCALE);
        pos_orig = t3d_vec3_scale(&obj->position_b, MODEL_SCALE);
        t3d_vec3_lerp(&pos, &pos_orig_old, &pos_orig, subtick);
        t3d_vec3_lerp(&roteul, &obj->rotation_euler_a,
                      &obj->rotation_euler_b, subtick);
        t3d_vec3_lerp(&scale, &obj->scale_a, &obj->scale_b, subtick);
        t3d_mat4fp_from_srt_euler(obj->matrix, scale.v, roteul.v, pos.v);
}

void object_destroy(struct object *obj)
{
        rspq_block_free(obj->displaylist);
        free_uncached(obj->matrix);
        t3d_model_free(obj->model);
}
