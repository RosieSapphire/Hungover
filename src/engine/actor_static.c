#include "engine/actor_static.h"

u8 actor_static_count = 0;
u8 actor_static_count_in_range = 0;
struct actor_static actor_statics[ACTOR_STATIC_MAX_COUNT];

struct actor_header *actor_static_init(FILE *file)
{
	const u8 ind = actor_static_count++;
	struct actor_header *h = (struct actor_header *)(actor_statics + ind);

	h->type = ACTOR_TYPE_STATIC;
	h->type_index = ind;

	char act_mdl_path[ACTOR_STATIC_MDLPATH_MAX_LEN];
	fread(act_mdl_path, 1, ACTOR_STATIC_MDLPATH_MAX_LEN, file);
	h->mdl = t3d_model_load(act_mdl_path);

	return h;
}

u8 actor_static_update(__attribute__((unused)) const u8 index,
		       __attribute__((unused))
		       const struct actor_update_params *params)
{
	return ACTOR_RETURN_NONE;
}
