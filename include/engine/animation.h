#ifndef _ENGINE_ANIMATION_H_
#define _ENGINE_ANIMATION_H_

#include "engine/types.h"
#include "engine/config.h"

typedef struct
{
	u16 frame;
	float vec[3];
} vec3_key_t;

typedef struct
{
	u16 frame;
	float vec[4];
} vec4_key_t;

typedef struct
{
	char name[CONF_NAME_MAX_LEN];
	u16 mesh_index, length, num_pos, num_rot, num_sca;
	vec3_key_t *pos;
	vec4_key_t *rot;
	vec3_key_t *sca;
	s16 frame, frame_last;
	bool is_playing;
	bool is_backward;
	bool loops;
} animation_t;

void animation_update(animation_t *a);
void animation_setup_matrix(const animation_t *a, float subtick);
void animation_destroy(animation_t *a);

#endif /* _ENGINE_ANIMATION_H_ */
