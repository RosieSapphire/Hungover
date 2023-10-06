#include "engine/sfx.h"

wav64_t title_music0, title_music1, title_music2,
       title_music3, title_music4, pickup_pistol, pickup_bong;

void sfx_init(void)
{
	/* channels */
	mixer_ch_set_vol(SFXC_MUSIC0, 1, 1);
	mixer_ch_set_vol(SFXC_MUSIC1, 0, 0);
	mixer_ch_set_vol(SFXC_MUSIC2, 0, 0);
	mixer_ch_set_vol(SFXC_MUSIC3, 1, 1);
	mixer_ch_set_vol(SFXC_MUSIC4, 0, 0);
	mixer_ch_set_vol(SFXC_ITEM, 1, 1);

	/* files */
	wav64_open(&title_music0, "rom:/title_intro.wav64");
	wav64_open(&title_music1, "rom:/title_init.wav64");
	wav64_open(&title_music2, "rom:/title_main.wav64");
	wav64_open(&title_music3, "rom:/title_play.wav64");
	wav64_open(&title_music4, "rom:/title_options.wav64");

	wav64_open(&pickup_pistol, "rom:/pickup_pistol.wav64");
	wav64_open(&pickup_bong, "rom:/pickup_bong.wav64");

	/* looping */
	wav64_set_loop(&title_music0, false);
	wav64_set_loop(&title_music1, true);
	wav64_set_loop(&title_music2, true);
	wav64_set_loop(&title_music3, false);
	wav64_set_loop(&title_music4, true);

	wav64_set_loop(&pickup_pistol, false);
	wav64_set_loop(&pickup_bong, false);
}
