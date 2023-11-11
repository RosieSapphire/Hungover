#ifndef GAME_TITLE_H
#define GAME_TITLE_H

#include <libdragon.h>

#include "engine/scene.h"
#include "engine/update.h"

/*
 * Main
 */
void title_load(void);
void title_unload(void);
enum scene_index title_update(struct update_parms uparms);
void title_draw(float subtick);

/*
 * Logo
 */
u8 title_logo_transform_intro(f32 *beats_lerp);
u8 title_logo_transform_main(const f32 music_t, const u8 music_ch_last);
u8 title_logo_transform_start(const f32 beats_lerp, f32 *difft,
			      const u8 music_state_last);
u8 title_logo_transform(const u8 music_state, const u8 music_state_last,
			const u8 music_ch_last, f32 *beats_lerp,
			f32 *difft, f32 music_t);
void title_logo_draw_object(const f32 difft, const u8 i, const u8 num_it,
			    const f32 beats_lerp, const f32 t,
			    const smesh_t *text_mesh, const u32 tid,
			    u8 *bg_is_white);
void title_logo_draw(const smesh_t *mesh, const u32 tid, f32 music_t, f32 t,
		     f32 subtick, f32 music_beats_last, f32 music_beats,
		     u8 music_state_last, u8 music_state,
		     const u8 music_ch_last, u8 *bg_is_white);

#endif /* GAME_TITLE_H */
