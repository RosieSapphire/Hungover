#ifndef _VERTEX_H_
#define _VERTEX_H_

#include "../../../include/engine/types.h"

/**
 * struct vertex - Vertex Struct
 * @pos: Position Component
 * @uv: Texture Coord Component
 */
struct vertex
{
	f32 pos[3];
	f32 uv[2];
};

#endif /* _VERTEX_H_ */
