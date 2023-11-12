#ifndef _ENGINE_SFX_H_
#define _ENGINE_SFX_H_

#include <libdragon.h>

/**
 * enum sfx_channels - SFX Channels
 * @SFXC_MUSIC0: Music Channel 0
 * @SFXC_MUSIC1: Music Channel 1
 * @SFXC_MUSIC2: Music Channel 2
 * @SFXC_MUSIC3: Music Channel 3
 * @SFXC_MUSIC4: Music Channel 4
 * @SFXC_ITEM: Item Channel
 * @SFXC_COUNT: Number of SFX Channels
 */
enum sfx_channels
{
	SFXC_MUSIC0,
	SFXC_MUSIC1,
	SFXC_MUSIC2,
	SFXC_MUSIC3,
	SFXC_MUSIC4,
	SFXC_ITEM,
	SFXC_COUNT,
};

extern wav64_t title_music0, title_music1, title_music2, title_music3,
	title_music4, pickup_pistol, pickup_bong, fire_pistol, lighter_bong;

void sfx_init(void);

#endif /* _ENGINE_SFX_H_ */
