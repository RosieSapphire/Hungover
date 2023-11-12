#ifndef ENGINE_SCENE_H_
#define ENGINE_SCENE_H_

#include "engine/mesh.h"
#include "engine/animation.h"
#include "engine/node.h"
#include "engine/update.h"

/**
 * enum scene_index - Scene Index
 * @SCENE_TITLE: Title Screen
 * @SCENE_TESTROOM: Test Room
 * @SCENE_COUNT: Numer of Scenes
 *
 * Description: Which scene we are currently interacting with
 */
enum scene_index
{
	SCENE_TITLE,
	SCENE_TESTROOM,
	SCENE_COUNT,
};

extern enum scene_index scene_index;
extern u32 pickup_spin_frame, pickup_spin_frame_last;

/**
 * struct scene - Scene Structure
 * @num_meshes: Number of Meshes
 * @meshes: Meshes Array
 * @num_anims: Number of Animations
 * @anims: Animations Array
 * @root_node: Root Node for Hierarchy
 * @num_tex_indis: Number of Texture Indices
 * @tex_indis: Texture Indices Array
 */
struct scene
{
	u16 num_meshes;
	struct mesh *meshes;
	u16 num_anims;
	struct animation *anims;
	struct node root_node;
	u16 num_tex_indis;
	u32 *tex_indis;
};

struct scene *scene_load(const char *path);
void scene_unload(struct scene *s);
void scene_update(struct scene *s);
void scene_draw(const struct scene *s, float subtick);
struct node *scene_node_from_name(struct node *n, const char *name);

#endif /* ENGINE_SCENE_H_ */
