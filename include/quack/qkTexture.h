#pragma once

#include <SDL3/SDL.h>
#ifdef SIMD_ENABLE
#include <immintrin.h>
#endif
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
#ifdef SIMD_ENABLE
void qkTextureSample8(const qkTexture* pTex, __m256 u, __m256 v, __m256i* pOut);
#endif