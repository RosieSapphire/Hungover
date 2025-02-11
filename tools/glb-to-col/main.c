#define IS_USING_SCENE_CONVERTER

#define T3D_RAD_TO_DEG(deg) (deg * 57.29577951289617186798f)

typedef struct {
	float v[3];
} T3DVec3;

typedef struct {
	float v[4];
} T3DQuat;

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <assimp/scene.h>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>

#include "config.h"
#include "engine/scene.h"

static void exitf(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);

	exit(EXIT_FAILURE);
}

static void triangleCalcNormal(CollisionTriangle *tri)
{
	float a[3] = {
		tri->verts[1].pos[0] - tri->verts[0].pos[0],
		tri->verts[1].pos[1] - tri->verts[0].pos[1],
		tri->verts[1].pos[2] - tri->verts[0].pos[2],
	};

	float b[3] = {
		tri->verts[2].pos[0] - tri->verts[0].pos[0],
		tri->verts[2].pos[1] - tri->verts[0].pos[1],
		tri->verts[2].pos[2] - tri->verts[0].pos[2],
	};

	tri->norm[0] = a[1] * b[2] - a[2] * b[1];
	tri->norm[1] = a[2] * b[0] - a[0] * b[2];
	tri->norm[2] = a[0] * b[1] - a[1] * b[0];

	float mag = sqrtf(tri->norm[0] * tri->norm[0] +
			  tri->norm[1] * tri->norm[1] +
			  tri->norm[2] * tri->norm[2]);

	if (!mag) {
		return;
	}

	tri->norm[0] /= mag;
	tri->norm[1] /= mag;
	tri->norm[2] /= mag;
}

static CollisionMesh collisionMeshFromAssimp(const struct aiScene *aiscn,
					     const struct aiNode *ainode)
{
	CollisionMesh ret;

	uint16_t numMeshes = ainode->mNumMeshes;
	CollisionMesh *meshesOut = calloc(numMeshes, sizeof *meshesOut);
	for (int i = 0; i < numMeshes; i++) {
		const struct aiMesh *in = aiscn->mMeshes[ainode->mMeshes[i]];
		CollisionMesh *out = meshesOut + i;

		out->numTriangles = in->mNumFaces;
		out->triangles =
			calloc(out->numTriangles, sizeof *out->triangles);
		for (uint16_t i = 0; i < out->numTriangles; i++) {
			const struct aiFace *triIn = in->mFaces + i;
			CollisionTriangle *triOut = out->triangles + i;

			for (uint16_t j = 0; j < 3; j++) {
				CollisionVertex *vertOut = triOut->verts + j;
				const struct aiVector3D vertIn =
					in->mVertices[triIn->mIndices[j]];

				vertOut->pos[0] = vertIn.x * T3DM_TO_N64_SCALE;
				vertOut->pos[1] = vertIn.y * T3DM_TO_N64_SCALE;
				vertOut->pos[2] = vertIn.z * T3DM_TO_N64_SCALE;
			}
			triangleCalcNormal(triOut);
		}
	}

	ret.numTriangles = 0;
	ret.triangles = malloc(0);
	for (uint16_t i = 0; i < numMeshes; i++) {
		const CollisionMesh *m = meshesOut + i;

		ret.numTriangles += m->numTriangles;
		ret.triangles =
			realloc(ret.triangles,
				sizeof *ret.triangles * ret.numTriangles);
		memcpy(ret.triangles + (ret.numTriangles - m->numTriangles),
		       m->triangles, sizeof *m->triangles * m->numTriangles);
	}
	ret.offset.v[0] = ainode->mTransformation.a4 * T3DM_TO_N64_SCALE;
	ret.offset.v[1] = ainode->mTransformation.b4 * T3DM_TO_N64_SCALE;
	ret.offset.v[2] = ainode->mTransformation.c4 * T3DM_TO_N64_SCALE;

	return ret;
}

static unsigned long fwriteEF16(const uint16_t *ptr, FILE *file)
{
	uint16_t flip = ((*ptr & 0x00FF) << 8) | ((*ptr & 0xFF00) >> 8);

	return fwrite(&flip, 2, 1, file);
}

static unsigned long fwriteEF32(const float *ptr, FILE *file)
{
	uint32_t bytes = *((uint32_t *)ptr);

	bytes = ((bytes & 0x000000FF) << 24) | ((bytes & 0x0000FF00) << 8) |
		((bytes & 0x00FF0000) >> 8) | ((bytes & 0xFF000000) >> 24);

	return fwrite(((float *)&bytes), 4, 1, file);
}

#ifdef RUN_TEST
static unsigned long freadEF16(uint16_t *ptr, FILE *file)
{
	unsigned long ret = fread(ptr, 2, 1, file);
	uint16_t bytes = *((uint16_t *)ptr);

	bytes = ((bytes & 0x00FF) << 8) | ((bytes & 0xFF00) >> 8);
	*ptr = *((uint16_t *)&bytes);

	return ret;
}

static unsigned long freadEF32(float *ptr, FILE *file)
{
	unsigned long ret = fread(ptr, sizeof *ptr, 1, file);
	uint32_t bytes = *((uint32_t *)ptr);

	bytes = ((bytes & 0x000000FF) << 24) | ((bytes & 0x0000FF00) << 8) |
		((bytes & 0x00FF0000) >> 8) | ((bytes & 0xFF000000) >> 24);
	*ptr = *((float *)&bytes);

	return ret;
}
#endif

static void collisionMeshWriteToFile(const CollisionMesh *cm, FILE *file)
{
	fwriteEF16(&cm->numTriangles, file);
	for (uint16_t i = 0; i < cm->numTriangles; i++) {
		CollisionTriangle *tri = cm->triangles + i;

		for (uint16_t j = 0; j < 3; j++) {
			for (uint16_t k = 0; k < 3; k++) {
				fwriteEF32(tri->verts[j].pos + k, file);
			}
		}

		for (uint16_t j = 0; j < 3; j++) {
			fwriteEF32(tri->norm + j, file);
		}
	}

	for (uint16_t i = 0; i < 3; i++) {
		fwriteEF32(cm->offset.v + i, file);
	}
}

static void quaternionFromMatrix(float quat[4], const float matrix[4][4])
{
	float r, rinv;
	float trace = matrix[0][0] + matrix[1][1] + matrix[2][2];
	if (trace >= 0.0f) {
		r = sqrtf(1.0f + trace);
		rinv = 0.5f / r;

		quat[0] = rinv * (matrix[1][2] - matrix[2][1]);
		quat[1] = rinv * (matrix[2][0] - matrix[0][2]);
		quat[2] = rinv * (matrix[0][1] - matrix[1][0]);
		quat[3] = r * 0.5f;
	} else if (matrix[0][0] >= matrix[1][1] &&
		   matrix[0][0] >= matrix[2][2]) {
		r = sqrtf(1.0f - matrix[1][1] - matrix[2][2] + matrix[0][0]);
		rinv = 0.5f / r;

		quat[0] = r * 0.5f;
		quat[1] = rinv * (matrix[0][1] + matrix[1][0]);
		quat[2] = rinv * (matrix[0][2] + matrix[2][0]);
		quat[3] = rinv * (matrix[1][2] - matrix[2][1]);
	} else if (matrix[1][1] >= matrix[2][2]) {
		r = sqrtf(1.0f - matrix[0][0] - matrix[2][2] + matrix[1][1]);
		rinv = 0.5f / r;

		quat[0] = rinv * (matrix[0][1] + matrix[1][0]);
		quat[1] = r * 0.5f;
		quat[2] = rinv * (matrix[1][2] + matrix[2][1]);
		quat[3] = rinv * (matrix[2][0] - matrix[0][2]);
	} else {
		r = sqrtf(1.0f - matrix[0][0] - matrix[1][1] + matrix[2][2]);
		rinv = 0.5f / r;

		quat[0] = rinv * (matrix[0][2] + matrix[2][0]);
		quat[1] = rinv * (matrix[1][2] + matrix[2][1]);
		quat[2] = r * 0.5f;
		quat[3] = rinv * (matrix[0][1] - matrix[1][0]);
	}
}

static void objectWriteToFile(const Object *o, FILE *file)
{
	fwrite(o->name, 1, OBJECT_NAME_MAX_LENGTH, file);

	for (int i = 0; i < 3; i++) {
		fwriteEF32(o->position.v + i, file);
	}

	for (int i = 0; i < 4; i++) {
		fwriteEF32(o->rotation.v + i, file);
	}

	for (int i = 0; i < 3; i++) {
		fwriteEF32(o->scale.v + i, file);
	}
}

static void areaWriteToFile(const Area *a, FILE *file)
{
	for (int i = 0; i < 3; i++) {
		fwriteEF32(a->offset.v + i, file);
	}
	collisionMeshWriteToFile(&a->colmesh, file);
	fwriteEF16(&a->numObjects, file);
	for (uint16_t i = 0; i < a->numObjects; i++) {
		objectWriteToFile(a->objects + i, file);
	}
}

static void sceneWriteToFile(const Scene *scn, FILE *file)
{
	fwriteEF16(&scn->numAreas, file);
	for (uint16_t i = 0; i < scn->numAreas; i++) {
		areaWriteToFile(scn->areas + i, file);
	}
	printf("Successfully wrote scene to file\n");
}

static void sceneProcessArea(Area *a, const struct aiScene *aiscn,
			     const struct aiNode *n)
{
	a->numObjects = 0;
	a->objects = malloc(0);
	a->offset.v[0] = n->mTransformation.a4 * T3DM_TO_N64_SCALE;
	a->offset.v[1] = n->mTransformation.b4 * T3DM_TO_N64_SCALE;
	a->offset.v[2] = n->mTransformation.c4 * T3DM_TO_N64_SCALE;
	int numColMeshes = 0;
	for (unsigned int i = 0; i < n->mNumChildren; i++) {
		const struct aiNode *ch = n->mChildren[i];

		if (!strncmp(ch->mName.data, "Col", 3)) {
			a->colmesh = collisionMeshFromAssimp(aiscn, ch);
			numColMeshes++;
			if (numColMeshes > 1) {
				exitf("ERROR: Area can only have "
				      "one collision mesh\n");
			}
		} else if (!strncmp(ch->mName.data, "Obj", 3)) {
			a->objects =
				realloc(a->objects,
					sizeof *a->objects * ++a->numObjects);
			Object *onew = a->objects + a->numObjects - 1;
			strncpy(onew->name, ch->mName.data,
				OBJECT_NAME_MAX_LENGTH);
			onew->position.v[0] =
				ch->mTransformation.a4 * T3DM_TO_N64_SCALE;
			onew->position.v[1] =
				ch->mTransformation.b4 * T3DM_TO_N64_SCALE;
			onew->position.v[2] =
				ch->mTransformation.c4 * T3DM_TO_N64_SCALE;
			const float matrix[4][4] = {
				{ ch->mTransformation.a1,
				  ch->mTransformation.b1,
				  ch->mTransformation.c1,
				  ch->mTransformation.d1 },

				{ ch->mTransformation.a2,
				  ch->mTransformation.b2,
				  ch->mTransformation.c2,
				  ch->mTransformation.d2 },

				{ ch->mTransformation.a3,
				  ch->mTransformation.b3,
				  ch->mTransformation.c3,
				  ch->mTransformation.d3 },

				{ ch->mTransformation.a4,
				  ch->mTransformation.b4,
				  ch->mTransformation.c4,
				  ch->mTransformation.d4 },
			};
			quaternionFromMatrix(onew->rotation.v, matrix);
			onew->scale = (T3DVec3){ {
				ch->mTransformation.a1,
				ch->mTransformation.b2,
				ch->mTransformation.c3,
			} };
		} else {
			exitf("ERROR: Invalid Item Type.\n");
		}
	}
}

static void assimpSceneProcessNode(Scene *scn, const struct aiScene *aiscn,
				   const struct aiNode *n)
{
	if (!strncmp("Area", n->mName.data, 4)) {
		scn->areas = realloc(scn->areas,
				     sizeof *scn->areas * ++scn->numAreas);
		sceneProcessArea(scn->areas + scn->numAreas - 1, aiscn, n);
	}

	for (unsigned int i = 0; i < n->mNumChildren; i++) {
		assimpSceneProcessNode(scn, aiscn, n->mChildren[i]);
	}
}

static void sceneDebug(const Scene *s, const char *scenePath)
{
	printf("DEBUGGING SCENE '%s' (%d areas)\n", scenePath, s->numAreas);
	for (uint16_t i = 0; i < s->numAreas; i++) {
		Area *a = s->areas + i;

		printf("\tArea %d (%d objects):\n", i, a->numObjects);
		printf("\t\tOffset: (%f, %f, %f)\n", a->offset.v[0],
		       a->offset.v[1], a->offset.v[2]);
		printf("\t\tCollision Mesh (%d triangles)\n",
		       a->colmesh.numTriangles);
		printf("\t\t\tOffset: (%f, %f, %f)\n", a->colmesh.offset.v[0],
		       a->colmesh.offset.v[1], a->colmesh.offset.v[2]);
		for (uint16_t j = 0; j < a->colmesh.numTriangles; j++) {
			CollisionTriangle *tri = a->colmesh.triangles + j;

			printf("\t\t\tTri %d:\n", j);
			for (uint16_t k = 0; k < 3; k++) {
				printf("\t\t\t\tPos %d: (%f, %f, %f)\n", k,
				       tri->verts[k].pos[0],
				       tri->verts[k].pos[1],
				       tri->verts[k].pos[2]);
			}
			printf("\t\t\t\tNorm: (%f, %f, %f)\n", tri->norm[0],
			       tri->norm[1], tri->norm[2]);
		}

		for (uint16_t j = 0; j < a->numObjects; j++) {
			Object *o = a->objects + j;

			printf("\t\tObject %d ('%s'):\n", j, o->name);
			printf("\t\t\tPosition: (%f, %f, %f):\n",
			       o->position.v[0], o->position.v[1],
			       o->position.v[2]);
			printf("\t\t\tRotation: (%f, %f, %f, %f):\n",
			       o->rotation.v[0], o->rotation.v[1],
			       o->rotation.v[2], o->rotation.v[3]);
			printf("\t\t\tScale: (%f, %f, %f):\n", o->scale.v[0],
			       o->scale.v[1], o->scale.v[2]);
		}
	}
}

static Scene handleScene(const struct aiScene *aiscn, const char *scenePath)
{
	Scene scn;
	scn.numAreas = 0;
	scn.areas = malloc(0);

	assimpSceneProcessNode(&scn, aiscn, aiscn->mRootNode);
	sceneDebug(&scn, scenePath);

	FILE *sceneFile = fopen(scenePath, "wb");

	if (!sceneFile) {
		exitf("Failed to write scene to '%s'\n", scenePath);
	}

	sceneWriteToFile(&scn, sceneFile);

	fclose(sceneFile);

	return scn;
}

int main(int argc, char **argv)
{
	char scenePath[512];
	const char *glbPath = argv[1];

	memset(scenePath, 0, 512);

	switch (argc) {
	default:
		exitf("Invalid number of arguments.\n");
		return 1;

	case 2:
		snprintf(scenePath, strlen(glbPath) - 3, "%s", glbPath);
		snprintf(scenePath + strlen(scenePath), 5, ".col");
		break;

	case 3:
		strncpy(scenePath, argv[2], 512);
		break;
	}

	const struct aiScene *scene =
		aiImportFile(glbPath, aiProcess_Triangulate |
					      aiProcess_JoinIdenticalVertices);

	if (!scene) {
		exitf("Failed to load scene from '%s'\n", glbPath);
	}

	handleScene(scene, scenePath);

	return 0;
}
