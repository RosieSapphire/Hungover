#ifndef _ANIMATION_H_
#define _ANIMATION_H_

#include <stdio.h>
#include "../../../include/engine/types.h"
#include "../../../include/engine/config.h"

/**
 * struct vec3_key - Vector 3 Keyframe
 * @frame: Frame
 * @vec: Vector Value
 */
struct vec3_key
{
	u16 frame;
	float vec[3];
};

/**
 * struct vec4_key - Vector 4 Keyframe
 * @frame: Frame
 * @vec: Vector Value
 */
struct vec4_key
{
	u16 frame;
	float vec[4];
};

/**
 * struct animation - Animation Structure
 * @name: Name of Animation
 * @mesh_index: Mesh Index to Animate
 * @length: Length of Animation in Frames
 * @num_pos: Number of Position Keyframes
 * @num_rot: Number of Rotation Keyframes
 * @num_sca: Number of Scaling Keyframes
 * @pos: Position Keyframes
 * @rot: Rotation Keyframes
 * @sca: Scaling Keyframes
 */
struct animation
{
	char name[CONF_NAME_MAX_LEN];
	u16 mesh_index, length, num_pos, num_rot, num_sca;
	struct vec3_key *pos;
	struct vec4_key *rot;
	struct vec3_key *sca;
};

void anim_read(struct animation *a, FILE *file);
void anim_write(const struct animation *a, FILE *file);

#endif /* _ANIMATION_H_ */
