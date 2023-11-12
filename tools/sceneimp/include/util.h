#ifndef _UTIL_H_
#define _UTIL_H_

#include "../../../include/engine/types.h"

u16 u16_endian_flip(u16 x);
f32 f32_endian_flip(f32 x);
void print_usage(char *argv0);
void matrix_transpose(f32 *in, f32 *out);
void matrix_print(f32 *mat, int indents);

#endif /* _UTIL_H_ */
