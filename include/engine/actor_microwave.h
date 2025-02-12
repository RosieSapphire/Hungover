#ifndef _ENGINE_MICROWAVE_ACTOR_H_
#define _ENGINE_MICROWAVE_ACTOR_H_

#include "types.h"

#include "engine/actor.h"

enum {
	MICROWAVE_STATE_IDLE,
	MICROWAVE_STATE_COOKING,
	MICROWAVE_STATE_FOOD_DONE,
};

#define MICROWAVE_COOK_TIME 5.f
#define MICROWAVE_BEEP_INTERVAL 1.f
#define MICROWAVE_NUM_BEEPS_WHEN_DONE 4

#define ACTOR_MICROWAVE_COUNT_MAX 1

struct actor_microwave {
	struct actor_header header;
	u8 state;
	u8 state_old;
	u8 beep_count;
	f32 beep_timer;
	f32 cook_time_left;
};

extern u8 actor_microwave_count_in_range;
extern u8 actor_microwave_count;
extern struct actor_microwave actor_microwaves[ACTOR_MICROWAVE_COUNT_MAX];

struct actor_header *actor_microwave_init(void);
u8 actor_microwave_update(const u8 index,
			  const struct actor_update_params *params);

#endif /* _ENGINE_MICROWAVE_ACTOR_H_ */
