#pragma once

#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>

struct object {
        T3DModel *model;
        T3DMat4FP *matrix;
        rspq_block_t *displaylist;
        T3DVec3 position_a;
        T3DVec3 position_b;
        T3DVec3 rotation_euler_a;
        T3DVec3 rotation_euler_b;
        T3DVec3 scale_a;
        T3DVec3 scale_b;
        void (*update_function)(struct object *, const float);
};

struct object object_create(const char *mdl_path, const T3DVec3 *pos,
                            const T3DVec3 *rot_eul, const T3DVec3 *scl,
                            void (*update_func)(struct object *, const float));
void object_setup_matrix(struct object *obj, const float subtick);
void object_destroy(struct object *obj);
