#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#ifdef WIN32
#include <Windows.h>
#endif

#include "Matrix.h"



void EngineInit();
void EngineRun();
void EngineQuit();

typedef unsigned char byte;

#define MESH_VERSION 100

struct MeshHeader
{
	unsigned short Version;
	unsigned short NumVertices;
};

typedef struct
{
	int Width;
	int Height;
	void* NativeHandle;
} Texture;

typedef struct
{
	Texture* Diffuse;
	byte R, G, B;

	bool IsLit;
	bool DepthWrite;
} Material;

struct Vector3
{
	float X, Y, Z;
};

struct Vector2
{
	float X, Y;
};

struct MeshVertex
{
	Vector3 Position;
	Vector2 UV;
};

struct Mesh
{
	int NumVertices;
	MeshVertex* Vertices;
};

struct Camera
{
	Vector3 Position;
	Vector3 Rotation;

	bool IsFreeLook;
};

Mesh* MeshLoad(char* fileName);
void MaterialSetGeneric(Material* material);

void GraphicsInit();
Camera* GraphicsGetCamera();
Texture* GraphicsCreateTextureFromFile(char* fileName);
void GraphicsBeginScene();
void GraphicsDrawTriangle(MeshVertex* verts, float* matrix);
void GraphicsDrawMesh(Mesh* mesh, Material* material, Vector3* position, Vector3* rotation);
void GraphicsDrawMeshEx(Mesh* mesh, Material* material, float* matrix);
void GraphicsDrawTexture(Texture* tex, float x, float y, float width, float height);
void GraphicsEndScene();

struct SoundBuffer
{
	byte* Buffer;
	int Length;

	bool IsPlaying;
};

void SoundInit();

