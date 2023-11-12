#ifndef _SCENE_H_
#define _SCENE_H_

#include <assimp/scene.h>
#include "../../../include/engine/types.h"

#include "node.h"

#define NAME_MAX_LEN 32
#define MAX_SCENE_TEXS 8
#define TEX_PATH_MAX_LEN 64

/**
 * struct scene - Scene Structure
 * @num_meshes: Number of Meshes
 * @meshes: Meshes Array
 * @num_anims: Number of Animations
 * @anims: Animations Array
 * @root_node: Root Node for Hierarchy
 * @num_tex_paths: Number of Texture Paths
 * @tex_paths: Texture Paths Array
 */
struct scene
{
	u16 num_meshes;
	struct mesh *meshes;
	u16 num_anims;
	struct animation *anims;
	struct node root_node;
	u16 num_tex_paths;
	char tex_paths[MAX_SCENE_TEXS][TEX_PATH_MAX_LEN];
};

/*
 * Main
 */
void scene_write(const struct scene *s, const char *outpath);
void scene_read_test(const char *path_out);
void scene_flush(struct scene *s);

/*
 * Processing
 */
void mesh_process(struct mesh *mo, const struct aiMesh *mi);
void node_process(struct node *no, const struct aiNode *ni);
void anim_process(struct animation *ao, const struct scene *so,
		const struct aiAnimation *ai);
void scene_process(struct scene *so, const struct aiScene *si);

#endif /* _SCENE_H_ */
