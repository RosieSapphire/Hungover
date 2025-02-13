#include "util.h"
#include "import.h"

#include <stdio.h>
#include <string.h>

#define inline
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#undef inline

extern int snprintf(char *str, size_t size, const char *format, ...);

const struct aiScene *assimp_scene_import(const int argc, const char **argv,
					  char scene_path[512])
{
	const int aiflags = aiProcess_Triangulate |
			    aiProcess_JoinIdenticalVertices;
	const char *aiscene_path = argv[1];

	if (strncmp(strrchr(aiscene_path, '/') + 1, "Scn.", 4)) {
		/* This is not a scene and we can safely skip it */
		return 0;
	}

	memset(scene_path, 0, 512);

	switch (argc) {
	default:
		exitf("Invalid number of arguments.\n");
		return NULL;

	case 2:
		snprintf(scene_path, strlen(aiscene_path) - 3, "%s",
			 aiscene_path);
		snprintf(scene_path + strlen(scene_path), 5, ".col");
		return aiImportFile(aiscene_path, aiflags);

	case 3:
		strncpy(scene_path, argv[2], 512);
		return aiImportFile(aiscene_path, aiflags);
	}

	return NULL;
}
