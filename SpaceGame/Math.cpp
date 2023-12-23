#include "Matrix.h"
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <math.h>

#define DEGTORAD 0.0174533f

// Value will be computed at compile time
#define MATRIX_SET(matrix, row, column, val) matrix[row * 4 + column] = val
#define MATRIX_GET(matrix, row, column) matrix[row * 4 + column]

void MatrixIdentity(float* matrix)
{
	memset(matrix, 0, sizeof(float) * 16);
	MATRIX_SET(matrix, 0, 0, 1);
	MATRIX_SET(matrix, 1, 1, 1);
	MATRIX_SET(matrix, 2, 2, 1);
	MATRIX_SET(matrix, 3, 3, 1);
}

void MatrixTranslation(float* matrix, float x, float y, float z)
{
	float m2[16];
	MatrixIdentity((float*)&m2);

	MATRIX_SET(m2, 3, 0, x);
	MATRIX_SET(m2, 3, 1, y);
	MATRIX_SET(m2, 3, 2, z);

	MatrixMultiply(matrix, (float*)&m2);
}

void MatrixRotationX(float* matrix, float rot)
{
	float m2[16];
	MatrixIdentity((float*)&m2);

	// TODO: Optimize it
	MATRIX_SET(m2, 1, 1, (float)cos(rot));
	MATRIX_SET(m2, 1, 2, (float)sin(rot));
	MATRIX_SET(m2, 2, 1, -(float)sin(rot));
	MATRIX_SET(m2, 2, 2, (float)cos(rot));

	MatrixMultiply(matrix, (float*)&m2);
}

void MatrixRotationY(float* matrix, float rot)
{
	float m2[16];
	MatrixIdentity((float*)&m2);

	// TODO: Optimize it
	MATRIX_SET(m2, 0, 0, (float)cos(rot));
	MATRIX_SET(m2, 0, 2, -(float)sin(rot));
	MATRIX_SET(m2, 2, 0, (float)sin(rot));
	MATRIX_SET(m2, 2, 2, (float)cos(rot));

	MatrixMultiply(matrix, (float*)&m2);
}

void MatrixRotationZ(float* matrix, float rot)
{
	float m2[16];
	MatrixIdentity((float*)&m2);

	// TODO: Optimize it
	MATRIX_SET(m2, 1, 1, (float)cos(rot));
	MATRIX_SET(m2, 1, 2, (float)sin(rot));
	MATRIX_SET(m2, 2, 1, -(float)sin(rot));
	MATRIX_SET(m2, 2, 2, (float)cos(rot));

	MatrixMultiply(matrix, (float*)&m2);
}

void MatrixRotationAngle(float* matrix, float x, float y, float z)
{
	MatrixRotationY(matrix, y * DEGTORAD);
}

void MatrixPrint(float* matrix)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
			printf("%f ", matrix[i * 4 + j]);

		printf("\n");
	}
}

void MatrixPerspective(float* matrix, float fov, float aspect, float _near, float _far)
{
	float m2[16];
	MatrixIdentity((float*)&m2);

	float yScale = 1.0f / (float)tan(fov / 2);
	float xScale = yScale / aspect;

	MATRIX_SET(m2, 0, 0, xScale);
	MATRIX_SET(m2, 1, 1, yScale);
	MATRIX_SET(m2, 2, 2, _far / (_far - _near));
	MATRIX_SET(m2, 3, 2, -_near * _far / (_far - _near));
	MATRIX_SET(m2, 3, 3, 0);
	MATRIX_SET(m2, 2, 3, 1);
	// Perspective FOV left-handed matrix

	MatrixMultiply(matrix, (float*)&m2);
}

void MatrixMultiply(float* matrix1, float* matrix2)
{
	float mat1[16];
	memcpy((float*)&mat1, matrix1, sizeof(float) * 16);

	for (int j = 0; j < 4; j++) {
		for (int i = 0; i < 4; i++) {
			MATRIX_SET(matrix1, j, i,
				MATRIX_GET(mat1, j, 0) * MATRIX_GET(matrix2, 0, i) +
				MATRIX_GET(mat1, j, 1) * MATRIX_GET(matrix2, 1, i) +
				MATRIX_GET(mat1, j, 2) * MATRIX_GET(matrix2, 2, i) +
				MATRIX_GET(mat1, j, 3) * MATRIX_GET(matrix2, 3, i));
		}
	}
}
