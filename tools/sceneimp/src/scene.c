#include <stdlib.h>
#include <errno.h>
#include <malloc.h>

#include "util.h"
#include "mesh.h"
#include "animation.h"
#include "scene.h"

/**
 * scene_write - Writes Scene to a File
 * @s: Scene in Question
 * @outpath: Output File Path
 */
void scene_write(const struct scene *s, const char *outpath)
{
	FILE *file = fopen(outpath, "wb");

	if (file)
	{
		fclose(file);
		remove(outpath);
		file = fopen(outpath, "wb");
	}
	else
	{
		fprintf(stderr, "Failed to write to file '%s' (%d) %s\n",
				outpath, errno, strerror(errno));
		exit(1);
	}

	int num_meshes_flip = u16_endian_flip(s->num_meshes);

	fwrite(&num_meshes_flip, sizeof(u16), 1, file);
	for (u16 i = 0; i < s->num_meshes; i++)
		mesh_write(s->meshes + i, file);

	int num_anims_flip = u16_endian_flip(s->num_anims);

	fwrite(&num_anims_flip, sizeof(u16), 1, file);
	for (u16 i = 0; i < s->num_anims; i++)
		anim_write(s->anims + i, file);

	node_write(&s->root_node, file);

	int num_tex_paths_flip = u16_endian_flip(s->num_tex_paths);

	fwrite(&num_tex_paths_flip, sizeof(u16), 1, file);
	for (u16 i = 0; i < s->num_tex_paths; i++)
		fwrite(s->tex_paths[i], sizeof(char), TEX_PATH_MAX_LEN, file);

	fclose(file);
}

/**
 * scene_read_test - Test out Reading a Scene from a File
 * @path_out: Path for Scene File
 */
void scene_read_test(const char *path_out)
{
	FILE *file = fopen(path_out, "rb");

	if (!file)
	{
		fprintf(stderr, "Failed to read file from '%s'\n", path_out);
		exit(1);
	}

	struct scene scene;

	fread(&scene.num_meshes, sizeof(u16), 1, file);
	scene.num_meshes = u16_endian_flip(scene.num_meshes);
	scene.meshes = malloc(sizeof(struct mesh) * scene.num_meshes);
	printf("num_meshes=%d\n", scene.num_meshes);
	for (int i = 0; i < scene.num_meshes; i++)
		mesh_read(scene.meshes + i, file);

	fread(&scene.num_anims, sizeof(u16), 1, file);
	scene.num_anims = u16_endian_flip(scene.num_anims);
	scene.anims = malloc(sizeof(struct animation) * scene.num_anims);
	printf("num_anims=%d\n", scene.num_anims);
	for (int i = 0; i < scene.num_anims; i++)
		anim_read(scene.anims + i, file);

	node_read(&scene.root_node, file, 0);

	fread(&scene.num_tex_paths, sizeof(u16), 1, file);
	scene.num_tex_paths = u16_endian_flip(scene.num_tex_paths);
	printf("num_tex_paths=%d\n", scene.num_tex_paths);
	for (u16 i = 0; i < scene.num_tex_paths; i++)
	{
		fread(scene.tex_paths[i], sizeof(char), TEX_PATH_MAX_LEN, file);
		printf("\ttex_path%d='%s'\n", i, scene.tex_paths[i]);
	}

	fclose(file);
}

/**
 * scene_flush - Flushes Scene Struct
 * @s: Scene in Question
 */
void scene_flush(struct scene *s)
{
	s->num_meshes = 0;
	s->num_anims = 0;
	free(s->meshes);
	free(s->anims);
}
