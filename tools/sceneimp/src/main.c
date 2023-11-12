#include <stdlib.h>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>

#include "util.h"
#include "scene.h"

/**
 * main - Main Function
 * @argc: Argument Count
 * @argv: Arguments Array
 *
 * Return: 0 = Success
 */
int main(int argc, char **argv)
{
	if (argc != 3)
		print_usage(argv[0]);

	const char *path_in = argv[1];
	const struct aiScene *scenein = aiImportFile(path_in,
			aiProcess_JoinIdenticalVertices |
			aiProcess_Triangulate |
			aiProcess_ImproveCacheLocality |
			aiProcess_RemoveRedundantMaterials |
			aiProcess_FlipUVs);

	if (!scenein)
	{
		fprintf(stderr, "Assimp Error (%s): %s\n", path_in,
				aiGetErrorString());
		exit(1);
	}

	const char *path_out = argv[2];
	struct scene sceneout;

	scene_process(&sceneout, scenein);
	scene_write(&sceneout, path_out);
	scene_flush(&sceneout);
	scene_read_test(path_out);

	return (0);
}
