#include <stdio.h>
#include <stdlib.h>

#include "util.h"

/**
 * u16_endian_flip - Unsigned 16-bit Integer Endian Flipper
 * @x: Value
 *
 * Return: Endian-Flipped Value
 */
u16 u16_endian_flip(u16 x)
{
	return ((x >> 8) | (x << 8));
}

/**
 * f32_endian_flip - 32-bit Floating Point Endian Flipper
 * @x: Value
 *
 * Return: Endian-Flipped Value
 */
f32 f32_endian_flip(f32 x)
{
	u32 tmp = *((u32 *)&x);

	tmp = ((tmp << 8) & 0xFF00FF00) | ((tmp >> 8) & 0x00FF00FF);
	tmp = (tmp << 16) | (tmp >> 16);

	return (*((f32 *)&tmp));
}

/**
 * print_usage - Prints out Usage Statement
 * @argv0: Just pass in 'argv[0]'
 */
void print_usage(char *argv0)
{
	fprintf(stderr, "Usage: %s [input .blend] [output .scn]\n", argv0);
	exit(EXIT_FAILURE);
}

/**
 * matrix_transpose - Transposes a 4x4 Floating Point Matrix
 * @in: Matrix Source
 * @out: Matrix Destination
 */
void matrix_transpose(f32 *in, f32 *out)
{
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			out[j * 4 + i] = in[i * 4 + j];
}

/**
 * matrix_print - Prints out a 4x4 Matrix
 * @mat: The Matrix
 * @indents: Number of Indents before Text
 */
void matrix_print(f32 *mat, int indents)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			for (int k = 0; k < indents; k++)
				printf("\t");

			printf("%f ", mat[j * 4 + i]);
		}
		printf("\n");
	}
	printf("\n");
}
