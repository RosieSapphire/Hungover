#ifndef IS_USING_SCENE_CONVERTER
#define IS_USING_SCENE_CONVERTER

/* specifically this program */
#include "util.h"
#include "import.h"
#include "convert.h"

/* from game */
#include "../../include/config.h"
#include "../../include/engine/scene.h"

int main(const int argc, const char **argv)
{
	char scene_path[512];
	const struct aiScene *aiscene;
	struct scene scene;

	if (!(aiscene = assimp_scene_import(argc, argv, scene_path))) {
		exitf("Failed to load aiscene from '%s'\n", scene_path);
	}
	scene = assimp_scene_to_scene(aiscene, scene_path);
	scene_debug(&scene, scene_path);

	return 0;
}

#endif /* IS_USING_SCENE_CONVERTER */
