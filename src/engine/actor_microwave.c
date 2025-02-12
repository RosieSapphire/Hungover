#include "input.h"

#include "engine/actor.h"
#include "engine/actor_microwave.h"
#include "engine/sfx.h"

// #define MICROWAVE_DEBUG
u8 actor_microwave_count_in_range;
u8 actor_microwave_count;
struct actor_microwave actor_microwaves[ACTOR_MICROWAVE_COUNT_MAX];

u8 actor_microwave_init(void)
{
	u8 ind = actor_microwave_count++;
	struct actor_microwave *mic = actor_microwaves + ind;

	mic->state = mic->state_old = MICROWAVE_STATE_IDLE;
	mic->cook_time_left = 0.f;
	mic->beep_timer = 0.f;
	mic->beep_count = 0;

	return ind;
}

u8 actor_microwave_update(struct actor_microwave *mic, const f32 dist,
			  const f32 dt)
{
	mic->state_old = mic->state;
	if (dist < 80.f) {
		actor_microwave_count_in_range++;
		if (INPUT_GET_BTN(A, PRESSED) &&
		    mic->state == MICROWAVE_STATE_IDLE) {
			mic->state = MICROWAVE_STATE_COOKING;
			mic->cook_time_left = MICROWAVE_COOK_TIME;
			sfx_play(SFX_MICROWAVE_RUNNING, 1.f);
		}
	}

	if (mic->cook_time_left > 0.f) {
		mic->cook_time_left -= dt;
		if (mic->cook_time_left <= 0.f) {
			mic->cook_time_left = 0.f;
			mic->state = MICROWAVE_STATE_FOOD_DONE;
			mic->beep_count = MICROWAVE_NUM_BEEPS_WHEN_DONE;
		}
	}

	if (mic->beep_count > 0) {
		if (mic->beep_timer > 0.f) {
			mic->beep_timer -= dt;
		} else {
			mic->beep_count--;
			mic->beep_timer = MICROWAVE_BEEP_INTERVAL;
			sfx_play(SFX_MICROWAVE_BEEP, 1.f);
		}
	} else if (mic->state == MICROWAVE_STATE_FOOD_DONE) {
		mic->state = MICROWAVE_STATE_IDLE;
		mic->beep_timer = 0.f;
	}

#ifdef MICROWAVE_DEBUG
	debugf("struct actor_microwave %d:\n", mic - actor_microwaves);
	debugf("\tstate=%d\n", mic->state);
	debugf("\tstate_old=%d\n", mic->state_old);
	debugf("\tcook_time_left=%f\n", mic->cook_time_left);
	debugf("\tbeep_count=%d\n", mic->beep_count);
	debugf("\tbeep_timer=%f\n", mic->beep_timer);
	debugf("\n");
#endif /* MICROWAVE_DEBUG */

	return ACTOR_RETURN_NONE;
}
