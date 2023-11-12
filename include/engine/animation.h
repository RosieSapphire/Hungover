#ifndef _ENGINE_ANIMATION_H_
#define _ENGINE_ANIMATION_H_

#include "engine/types.h"
#include "engine/config.h"

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
 * enum anim_flags - Animation Flags
 * @ANIM_FLAGS_NONE: Animation Empty Flags
 * @ANIM_IS_PLAYING: Animtion is Playing Flag
 * @ANIM_IS_BACKWARD: Animtion is Backward Flag
 * @ANIM_IS_LOOPING: Animtion is Looping Flag
 */
enum anim_flags
{
	ANIM_FLAGS_NONE  = 0x0,
	ANIM_IS_PLAYING  = 0x1,
	ANIM_IS_BACKWARD = 0x2,
	ANIM_IS_LOOPING  = 0x4,
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
 * @frame: Current Frame
 * @frame_last: Last Frame
 * @flags: Animation Flags
 */
struct animation
{
	char name[CONF_NAME_MAX_LEN];
	u16 mesh_index, length, num_pos, num_rot, num_sca;
	struct vec3_key *pos;
	struct vec4_key *rot;
	struct vec3_key *sca;
	s16 frame, frame_last;
	u8 flags;
};

void animation_update(struct animation *a);
void animation_setup_matrix(const struct animation *a, float subtick);
void animation_destroy(struct animation *a);

#endif /* _ENGINE_ANIMATION_H_ */
