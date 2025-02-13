#ifndef _GLTF_TO_SCN_CONVERT_H_
#define _GLTF_TO_SCN_CONVERT_H_

#ifndef IS_USING_SCENE_CONVERTER
#define IS_USING_SCENE_CONVERTER
#endif /* IS_USING_SCENE_CONVERTER */

struct aiScene;
struct aiNode;

#include "../../include/engine/scene.h"

struct scene assimp_scene_to_scene(const struct aiScene *aiscn,
				 const char *scene_path);
void assimp_scene_node_recursive(struct scene *scn,
				      const struct aiScene *aiscn,
				      const struct aiNode *ainode);
struct area assimp_node_to_area(const struct aiScene *aiscn,
				const struct aiNode *ainode);
struct collision_mesh assimp_node_to_collision_mesh(const struct aiScene *aiscn,
			      const struct aiNode *ainode);


#endif /* _GLTF_TO_SCN_CONVERT_H_ */
