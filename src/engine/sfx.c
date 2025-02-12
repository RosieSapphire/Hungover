#include <libdragon.h>

#include "engine/sfx.h"

static wav64_t sfx_samples[SFX_SAMPLE_COUNT];
static const char *sfx_paths[SFX_SAMPLE_COUNT] = {
	"rom:/Sfx.MicrowaveRunning.wav64", "rom:/Sfx.MicrowaveBeep.wav64"
};

void sfx_init(void)
{
	audio_init(44100, 4);
	mixer_init(1);

	for (u16 i = 0; i < SFX_SAMPLE_COUNT; i++) {
		wav64_open(sfx_samples + i, sfx_paths[i]);
	}
}

void sfx_play(const u16 sfx, const f32 vol)
{
	mixer_ch_set_vol(0, vol, vol);
	wav64_play(sfx_samples + sfx, 0);
}

void sfx_update(void)
{
	if (!audio_can_write()) {
		return;
	}

	short *audio_buf = audio_write_begin();
	mixer_poll(audio_buf, audio_get_buffer_length());
	audio_write_end();
}

void sfx_free(void)
{
	for (u16 i = 0; i < SFX_SAMPLE_COUNT; i++) {
		wav64_close(sfx_samples + i);
	}

	mixer_close();
	audio_close();
}
