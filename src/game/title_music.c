#include "engine/sfx.h"

#include "game/title.h"

#define MUSIC_BPM 178
#define MUSIC_DELTA ((MUSIC_BPM / 60.0f) / (f32)CONF_TICKRATE)

void title_music_change(struct update_parms uparms, u8 *music_state_last,
			u8 *music_state, const u8 opt)
{
	u8 msl = *music_state_last;
	u8 ms = *music_state;

	msl = ms;
	switch (ms)
	{
	case 0:
	case 1:
		ms += uparms.down.start;
		break;

	case 2:
		switch (opt)
		{
		case 0:
		case 1:
			ms += uparms.down.start || uparms.down.a;
			break;

		case 2:
			ms += uparms.down.start || uparms.down.a;
			break;
		}
		break;

	case 3:
		break;

	case 4:
		ms -= uparms.down.b * 2;
		break;
	}

	*music_state_last = msl;
	*music_state = ms;
}

void title_music_update(f32 *music_beats_last, f32 *music_beats,
			u8 *music_state_last, u8 *music_state,
			u8 *music_ch_last, u8 *music_ch_cur,
			f32 *music_state_timer_last, f32 *music_state_timer)
{
	f32 mbl = *music_beats_last;
	f32 mb = *music_beats;
	u8 msl = *music_state_last;
	u8 ms = *music_state;
	u8 mcl = *music_ch_last;
	u8 mc = *music_ch_cur;
	f32 mstl = *music_state_timer_last;
	f32 mst = *music_state_timer;

	mbl = mb;
	mb += MUSIC_DELTA;
	if (mb >= 33.0f && ms == 0)
		ms = 1;
	if (msl != ms)
	{
		mcl = mc;
		mc = ms;
		mst = 0.0f;

		if (mc == SFXC_MUSIC3)
		{
			mixer_ch_stop(SFXC_MUSIC0);
			mixer_ch_stop(SFXC_MUSIC1);
			mixer_ch_stop(SFXC_MUSIC2);
			mixer_ch_stop(SFXC_MUSIC4);
			wav64_play(&title_music3, SFXC_MUSIC3);
		}
	}

	mstl = mst;
	mst += 0.06f;

	*music_beats_last = mbl;
	*music_beats = mb;
	*music_state_last = msl;
	*music_state = ms;
	*music_ch_last = mcl;
	*music_ch_cur = mc;
	*music_state_timer_last = mstl;
	*music_state_timer = mst;
}

void title_music_volume(const f32 music_t, const u8 music_ch_last,
			const u8 music_ch_cur)
{
	mixer_ch_set_vol(music_ch_last, 1.0f - music_t, 1.0f - music_t);
	mixer_ch_set_vol(music_ch_cur, music_t, music_t);
	if (music_ch_last == SFXC_MUSIC0 || music_ch_last == SFXC_MUSIC2)
	{
		mixer_ch_set_vol(music_ch_last, 0, 0);
		mixer_ch_set_vol(music_ch_cur, 1, 1);
	}
}

