#ifndef _GLB_TO_SCN_IMPORT_H_
#define _GLB_TO_SCN_IMPORT_H_

const struct aiScene *assimp_scene_import(const int argc,
						     const char **argv,
						     char scene_path[512]);

#endif /* _GLB_TO_SCN_IMPORT_H_ */
