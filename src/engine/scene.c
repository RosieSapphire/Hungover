#include <stdio.h>
#include <malloc.h>

// tmp
#include <libdragon.h>

#include "engine/scene.h"

enum scene_index scene_index = SCENE_TITLE;

scene_t *scene_load(const char *path)
{
	FILE *file = fopen(path, "rb");
	scene_t *scene = malloc(sizeof(scene_t));
	fread(&scene->num_meshes, sizeof(uint16_t), 1, file);
	scene->meshes = malloc(sizeof(smesh_t) * scene->num_meshes);
	for(int i = 0; i < scene->num_meshes; i++) {
		smesh_t *mesh = scene->meshes + i;
		fread(&mesh->num_verts, sizeof(uint16_t), 1, file);
		mesh->verts = malloc(sizeof(vertex_t) * mesh->num_verts);
		for(int j = 0; j < mesh->num_verts; j++)
			fread(mesh->verts + j, sizeof(vertex_t), 1, file);

		fread(&mesh->num_indis, sizeof(uint16_t), 1, file);
		mesh->indis = malloc(sizeof(uint16_t) * mesh->num_indis);
		fread(mesh->indis, sizeof(uint16_t), mesh->num_indis, file);
	}
	fclose(file);
	// _scene_debug(scene);

	return scene;
}

void scene_unload(scene_t *s)
{
	for(int i = 0; i < s->num_meshes; i++)
		smesh_destroy(s->meshes);

	free(s);
}

void scene_draw(const scene_t *s, /* temp */const uint32_t tid)
{
	for(int i = 0; i < s->num_meshes; i++)
		smesh_draw(s->meshes + i, tid);
}
