#pragma once

// Misc matrix math

void MatrixIdentity(float* matrix);
void MatrixTranslation(float* matrix, float x, float y, float z);
void MatrixRotationAngle(float* matrix, float x, float y, float z);
void MatrixPerspective(float* matrix, float fov, float aspect, float _near, float _far);
void MatrixMultiply(float* matrix1, float* matrix2);