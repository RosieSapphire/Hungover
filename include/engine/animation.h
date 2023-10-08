#ifndef ENGINE_ANIMATION_H_
#define ENGINE_ANIMATION_H_

#include <stdint.h>

#include "engine/config.h"

typedef struct {
	uint16_t frame;
	float vec[3];
} vec3_key_t;

typedef struct {
	uint16_t frame;
	float vec[4];
} vec4_key_t;

typedef struct {
	char name[CONF_NAME_MAX_LEN];
	uint16_t mesh_index, length, num_pos, num_rot, num_sca;
	vec3_key_t *pos;
	vec4_key_t *rot;
	vec3_key_t *sca;
	uint16_t frame, frame_last;
	bool is_playing;
	bool loops;
} animation_t;

// void animation_debug(const animation_t *a);
void animation_update(animation_t *a);
void animation_setup_matrix(const animation_t *a, float subtick);
void animation_destroy(animation_t *a);

#endif /* ENGINE_ANIMATION_H_ */
