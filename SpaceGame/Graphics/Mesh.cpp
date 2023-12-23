#include <Engine.h>
#include <memory.h>

Mesh* MeshLoad(char* fileName)
{
	MeshHeader header;
	memset(&header, 0, sizeof(header));
	FILE* f = fopen(fileName, "rb");
	fread(&header, sizeof(header), 1, f);

	if (header.Version != MESH_VERSION)
	{
		printf("Mesh version mismatch for %s\n", fileName);

		return 0;
	}

	Mesh* ret = new Mesh();
	ret->NumVertices = header.NumVertices;
	ret->Vertices = new MeshVertex[header.NumVertices];
	fread(ret->Vertices, sizeof(MeshVertex), ret->NumVertices, f);
	fclose(f);

	return ret;
}