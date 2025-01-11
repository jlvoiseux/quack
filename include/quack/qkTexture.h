#pragma once

#include <SDL3/SDL.h>
#include <stdint.h>

typedef struct qkTexture
{
	uint32_t* pData;
	int		  width;
	int		  height;
} qkTexture;

int		 qkTextureLoad(const char* pFilename, qkTexture* pOut);
void	 qkTextureDestroy(qkTexture* pTex);
uint32_t qkTextureSample(const qkTexture* pTex, float u, float v);