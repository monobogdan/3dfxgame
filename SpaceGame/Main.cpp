#include <Windows.h>

#include <Engine.h>

void ShowError(char* msg)
{
	MessageBoxA(0, msg, "An error occured", MB_ICONERROR);
	TerminateProcess(GetCurrentProcess(), -1);
}


/*Bitmap* BitmapLoad(char* fileName)
{
	FILE* f = fopen(fileName, "rb");

	if (!f)
		return 0;

	TgaHeader hdr;
	fread(&hdr, sizeof(hdr), 1, f);

	int sz = sizeof(unsigned short);

	if (hdr.paletteType || hdr.bpp != 16)
		ShowError("Only 16-bit TGA bitmaps are supported. Use texus to create full set of mipmaps.");

	byte* buf = (byte*)malloc(hdr.width * hdr.height * (hdr.bpp / 8));
	assert(buf);

	fseek(f, hdr.headerLength, SEEK_SET);

	fread(buf, hdr.width * hdr.height * (hdr.bpp / 8), 1, f);


	fclose(f);
	Bitmap* ret = new Bitmap();
	ret->Pixels = buf;
	ret->Width = hdr.width;
	ret->Height = hdr.height;

	return ret;
}*/

int main(int argc, char** argv)
{
	GraphicsInit();

	Mesh* mesh = MeshLoad("tris.msh");

	Texture* tex = GraphicsCreateTextureFromFile("skin.3df");
	Material mat;
	MaterialSetGeneric(&mat);
	mat.Diffuse = tex;

	float identity[16];
	
	float rot = 15;

	while (true)
	{
		GraphicsBeginScene();

		rot -= 0.1f;

		MatrixIdentity((float*)&identity);
		MatrixRotationAngle((float*)&identity, 0, rot, 0);
		MatrixTranslation((float*)&identity, rot, 0, 15);
		MatrixPerspective((float*)&identity, 90 * 0.0174533f, 640.0f / 480, 1, 100);

		GraphicsDrawMeshEx(mesh, &mat, (float*)&identity);

		GraphicsEndScene();
	}
	
	return 0;
}