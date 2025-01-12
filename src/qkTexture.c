#include "quack/qkTexture.h"

#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int qkTextureLoad(const char* pFilename, qkTexture* pOut)
{
	int channels;
	pOut->pData = (uint32_t*)stbi_load(pFilename, &pOut->width, &pOut->height, &channels, 4);

	if (!pOut->pData)
	{
		return -1;
	}

	return 0;
}

void qkTextureDestroy(qkTexture* pTex)
{
	if (!pTex)
		return;

	if (pTex->pData)
	{
		stbi_image_free(pTex->pData);
		pTex->pData = NULL;
	}
}

uint32_t qkTextureSample(const qkTexture* pTex, float u, float v)
{
	int x = (int)(u * ((float)pTex->width - 1) + 0.5f);
	int y = (int)(v * ((float)pTex->height - 1) + 0.5f);

	x = (x % pTex->width + pTex->width) % pTex->width;
	y = (y % pTex->height + pTex->height) % pTex->height;

	return pTex->pData[y * pTex->width + x];
}