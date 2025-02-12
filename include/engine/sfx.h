#ifndef _ENGINE_SFX_H_
#define _ENGINE_SFX_H_

#include "types.h"

enum { SFX_MICROWAVE_RUNNING, SFX_MICROWAVE_BEEP, SFX_SAMPLE_COUNT };

void sfx_init(void);
void sfx_play(const u16 sfx, const f32 vol);
void sfx_update(void);
void sfx_free(void);

#endif /* _ENGINE_SFX_H_ */
