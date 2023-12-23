#include <Engine.h>

#include <glide.h>
#include <stdlib.h>
#include <stdio.h>

#define GR_VERTEX_X_OFFSET              0
#define GR_VERTEX_Y_OFFSET              1
#define GR_VERTEX_OOZ_OFFSET            2
#define GR_VERTEX_OOW_OFFSET            3
#define GR_VERTEX_R_OFFSET              4
#define GR_VERTEX_G_OFFSET              5
#define GR_VERTEX_B_OFFSET              6
#define GR_VERTEX_A_OFFSET              7
#define GR_VERTEX_Z_OFFSET              8
#define GR_VERTEX_SOW_TMU0_OFFSET       9
#define GR_VERTEX_TOW_TMU0_OFFSET       10
#define GR_VERTEX_OOW_TMU0_OFFSET       11
#define GR_VERTEX_SOW_TMU1_OFFSET       12
#define GR_VERTEX_TOW_TMU1_OFFSET       13
#define GR_VERTEX_OOW_TMU1_OFFSET       14
#if (GLIDE_NUM_TMU > 2)
#define GR_VERTEX_SOW_TMU2_OFFSET       15
#define GR_VERTEX_TOW_TMU2_OFFSET       16
#define GR_VERTEX_OOW_TMU2_OFFSET       17
#endif

typedef struct {
	float  U;                   /* s texture ordinate (s over w) */
	float  V;                   /* t texture ordinate (t over w) */
	float  oow;                   /* 1/w (used mipmapping - really 0xfff/w) */
}  GrTmuVertex;

typedef struct
{
	float X, Y;         /* X and Y in screen space */
	float Z;          /* 65535/Z (used for Z-buffering) */
	float Q;          /* 1/W (used for W-buffering, texturing) */
	float r, g, b, a;   /* R, G, B, A [0..255.0] */
	float Z2;            /* Z is ignored */
	GrTmuVertex  tmuvtx[GLIDE_NUM_TMU];
} Vertex;

typedef struct
{
	GrTexInfo TexInfo;
	FxU32 LinearAddr;
} NativeTextureHandle;

GrContext_t gContext;
int gWidth, gHeight;

FxU32 gTextureHeapMin, gTextureHeapMax;
FxU32 gTextureHeapUsage;

void GraphicsInit()
{
	int numBoards = 0;
	grGet(GR_NUM_BOARDS, sizeof(numBoards), (FxI32*)&numBoards);

	if (numBoards < 1)
	{
		printf("No Glide GPU is present\n");
		exit(-1);
	}

	printf("Initializing glide renderer\n");
	grGlideInit();
	grSstSelect(0);

	gContext = grSstWinOpen(0, GR_RESOLUTION_640x480, GR_REFRESH_60Hz, GR_COLORFORMAT_RGBA, GR_ORIGIN_UPPER_LEFT,
		2, 1); // RGBA, double buffered, 1 depth-buffer

	gWidth = 640;
	gHeight = 480;

	if (!gContext)
	{
		printf("Failed to initialize context\n");
		exit(-1);
	}

	grCoordinateSpace(GR_WINDOW_COORDS);
	grDepthBufferMode(GR_DEPTHBUFFER_ZBUFFER);
	grDepthBufferFunction(GR_CMP_GREATER);
	grDepthMask(FXTRUE);

	// Setup vertex format
	/*grVertexLayout(GR_PARAM_XY, 4, GR_PARAM_ENABLE);
	grVertexLayout(GR_PARAM_Q, 12, GR_PARAM_ENABLE);
	grVertexLayout(GR_PARAM_RGB, 16, GR_PARAM_ENABLE);
	grVertexLayout(GR_PARAM_ST0, 28, GR_PARAM_ENABLE);*/
	grVertexLayout(GR_PARAM_XY, GR_VERTEX_X_OFFSET << 2, GR_PARAM_ENABLE);
	grVertexLayout(GR_PARAM_RGB, GR_VERTEX_R_OFFSET << 2, GR_PARAM_ENABLE);
	grVertexLayout(GR_PARAM_A, GR_VERTEX_A_OFFSET << 2, GR_PARAM_ENABLE);
	grVertexLayout(GR_PARAM_Z, GR_VERTEX_OOZ_OFFSET << 2, GR_PARAM_ENABLE);
	grVertexLayout(GR_PARAM_Q, GR_VERTEX_OOW_OFFSET << 2, GR_PARAM_ENABLE);
	grVertexLayout(GR_PARAM_ST0, GR_VERTEX_SOW_TMU0_OFFSET << 2, GR_PARAM_ENABLE);
	grCullMode(GR_CULL_POSITIVE);

	printf("Context created with framebuffer size: %dx%d\n", gWidth, gHeight);
	printf("Glide version: %s\n", grGetString(GR_VENDOR));
	printf("Glide renderer: %s\n", grGetString(GR_RENDERER));
	printf("Glide version: %s\n", grGetString(GR_VERSION));

	grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
		GR_COMBINE_FACTOR_ONE,
		GR_COMBINE_LOCAL_NONE,
		GR_COMBINE_OTHER_CONSTANT,
		FXFALSE);
	grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL,
		GR_COMBINE_FACTOR_NONE,
		GR_COMBINE_LOCAL_CONSTANT,
		GR_COMBINE_OTHER_NONE,
		FXFALSE);
	grAlphaBlendFunction(GR_BLEND_ONE,
		GR_BLEND_ZERO,
		GR_BLEND_ZERO,
		GR_BLEND_ZERO);

	gTextureHeapMin = grTexMinAddress(GR_TMU0);
	gTextureHeapMax = grTexMaxAddress(GR_TMU0);
	gTextureHeapUsage = 0;
}

Camera* GraphicsGetCamera()
{
	return 0;
}

Texture* GraphicsCreateTextureFromFile(char* fileName)
{
	Gu3dfInfo info;
	gu3dfGetInfo(fileName, &info);

	FxU32 required = grTexCalcMemRequired(info.header.small_lod, info.header.large_lod, info.header.aspect_ratio, info.header.format);

	if (gTextureHeapMin + gTextureHeapUsage + required > gTextureHeapMax)
	{
		printf("Failed to allocate texture %s: VRAM exhausted\n", fileName);
		exit(-1);
	}

	FxU32 linearAddr = gTextureHeapMin + gTextureHeapUsage;

	info.data = malloc(info.mem_required);
	gu3dfLoad(fileName, &info);

	GrTexInfo tInfo;
	tInfo.aspectRatioLog2 = info.header.aspect_ratio;
	tInfo.smallLodLog2 = info.header.small_lod;
	tInfo.largeLodLog2 = info.header.large_lod;
	tInfo.format = info.header.format;
	tInfo.data = info.data;

	grTexDownloadMipMap(0, linearAddr, GR_MIPMAPLEVELMASK_BOTH, &tInfo);
	grTexSource(0, linearAddr, GR_MIPMAPLEVELMASK_BOTH, &tInfo);

	NativeTextureHandle* tex = new NativeTextureHandle();
	tex->TexInfo = tInfo;
	tex->LinearAddr = linearAddr;

	Texture* ret = new Texture();
	ret->Width = 0;
	ret->Height = 0;
	ret->NativeHandle = tex;

	return ret;
}

void GraphicsBeginScene()
{
	// Buffer clear operations aren't free on Voodoo. When scene will have skybox, comment clear
	grBufferClear(RGB(0, 128, 0), 0, 0);
}

void GraphicsEndScene()
{
	grFinish();
	grBufferSwap(1);
}

#define XVALUE_TO_WINDOW_SPACE(srcX, width) (width / 2) + (srcX * width);
#define YVALUE_TO_WINDOW_SPACE(srcY, width) width - ((width / 2) + (srcY * width));

// Project vertex from world space to clip space. matrix - MVP matrix.
// Near-clipping stupid technique: if W < 0, then discard whole triangle
__inline bool GraphicsProjectVertex(Vertex* vert, float* matrix)
{
	float m[4][4];
	memcpy(&m, matrix, sizeof(m)); // Temporary
	Vertex ret;

	// We treat vertex as float4(pos, 1)
	ret.X = vert->X  * m[0][0] + vert->Y  * m[1][0] + vert->Z * m[2][0] + (1 * m[3][0]);
	ret.Y = vert->X  * m[0][1] + vert->Y * m[1][1] + vert->Z * m[2][1] + (1 * m[3][1]);
	ret.Z = vert->X  * m[0][2] + vert->Y * m[1][2] + vert->Z * m[2][2] + (1 * m[3][2]);
	ret.Q = vert->X  * m[0][3] + vert->Y * m[1][3] + vert->Z * m[2][3] + (1 * m[3][3]);

	*vert = ret;

	return ret.Q >= 0;
}

float absF(float val)
{
	return val < 0 ? -val : val;
}

__inline bool GraphicsConvertToWindowSpace(Vertex* inV, Vertex* outV, float* matrix)
{
	Vertex v = *inV;
	if (!GraphicsProjectVertex(&v, (float*)matrix))
		return false;

	outV->X = XVALUE_TO_WINDOW_SPACE(v.X / v.Q, 640);
	outV->Y = YVALUE_TO_WINDOW_SPACE(v.Y / v.Q, 480);
	outV->Z = 65535.0f / absF(v.Z); // Due to LH coordinate system, forward is -Z. So, we need to invert Z-value to get right value
	outV->Q = 1 / v.Q;

	outV->tmuvtx[0].U = v.tmuvtx[0].U / v.Q;
	outV->tmuvtx[0].V = v.tmuvtx[0].V / v.Q;
	//outV->tmuvtx[0].oow = v.Q;

	return true;
}

void GraphicsDrawTriangle(MeshVertex* verts, float* matrix)
{
	if (verts)
	{
		// Convert generic vertex format to Glide format first
		Vertex v[3];
		memset(&v, 0, sizeof(v));

		for (int i = 0; i < 3; i++)
		{
			v[i].X = verts[i].Position.X;
			v[i].Y = verts[i].Position.Y;
			v[i].Z = verts[i].Position.Z;
			v[i].tmuvtx[0].U = verts[i].UV.X * 255.0f;
			v[i].tmuvtx[0].V = verts[i].UV.Y * 255.0f;
		}

		Vertex v1, v2, v3;
		if (!GraphicsConvertToWindowSpace(&v[0], &v1, matrix))
			return;

		if (!GraphicsConvertToWindowSpace(&v[1], &v2, matrix))
			return;

		if (!GraphicsConvertToWindowSpace(&v[2], &v3, matrix))
			return;

		grDrawTriangle(&v1, &v2, &v3);
	}
}

void GraphicsSetupMissingMaterial()
{
	grConstantColorValue(4278255615);

	/*grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
		GR_COMBINE_FACTOR_ONE,
		GR_COMBINE_LOCAL_ITERATED,
		GR_COMBINE_OTHER_CONSTANT,
		FXFALSE);*/
	/*grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL,
		GR_COMBINE_FACTOR_NONE,
		GR_COMBINE_LOCAL_CONSTANT,
		GR_COMBINE_OTHER_NONE,
		FXFALSE);
	grAlphaBlendFunction(GR_BLEND_ONE,
		GR_BLEND_ZERO,
		GR_BLEND_ZERO,
		GR_BLEND_ZERO);*/
}

void GraphicsSetupTextureStage(Material* mat)
{
	if (!mat->Diffuse)
	{
		GraphicsSetupMissingMaterial();

		return;
	}

	// Bind texture

	NativeTextureHandle* handle = (NativeTextureHandle*)mat->Diffuse->NativeHandle;
	

	grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
		GR_COMBINE_FACTOR_ONE,
		GR_COMBINE_LOCAL_NONE,
		GR_COMBINE_OTHER_TEXTURE,
		FXFALSE);
	grAlphaCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
		GR_COMBINE_FACTOR_ONE,
		GR_COMBINE_LOCAL_NONE,
		GR_COMBINE_OTHER_TEXTURE,
		FXFALSE);
	grAlphaBlendFunction(GR_BLEND_SRC_ALPHA,
		GR_BLEND_ONE_MINUS_SRC_ALPHA,
		GR_BLEND_ZERO,
		GR_BLEND_ZERO);

	grTexFilterMode(GR_TMU0,
		GR_TEXTUREFILTER_BILINEAR,
		GR_TEXTUREFILTER_BILINEAR);
	grTexMipMapMode(GR_TMU0,
		GR_MIPMAP_NEAREST,
		FXTRUE);
}

void MaterialSetGeneric(Material* material)
{
	material->R = 1;
	material->G = 1;
	material->B = 1;
	material->IsLit = true;
	material->DepthWrite = true;
	material->Diffuse = 0;
}

void GraphicsDrawMeshEx(Mesh* mesh, Material* material, float* matrix)
{
	if (mesh && mesh->NumVertices > 0)
	{
		// Setup render state
		GraphicsSetupTextureStage(material);

		for (int i = 0; i < mesh->NumVertices / 3; i++)
		{
			GraphicsDrawTriangle(&mesh->Vertices[i * 3], matrix);
		}
	}
}

void GraphicsDrawMesh(Mesh* mesh, Material* material, Vector3* position, Vector3* rotation)
{
	
}

void GraphicsDrawTexture(Texture* tex, float x, float y, float width, float height)
{

}