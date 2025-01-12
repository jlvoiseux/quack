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

#ifdef SIMD_ENABLE
void qkTextureSample8(const qkTexture* pTex, __m256 u, __m256 v, __m256i* pOut)
{
	const __m256 widthF	 = _mm256_set1_ps((float)pTex->width - 1);
	const __m256 heightF = _mm256_set1_ps((float)pTex->height - 1);

	const __m256 xf = _mm256_fmadd_ps(u, widthF, _mm256_set1_ps(0.5f));
	const __m256 yf = _mm256_fmadd_ps(v, heightF, _mm256_set1_ps(0.5f));

	__m256i x = _mm256_cvttps_epi32(xf);
	__m256i y = _mm256_cvttps_epi32(yf);

	const __m256i widthMask	 = _mm256_set1_epi32(pTex->width - 1);
	const __m256i heightMask = _mm256_set1_epi32(pTex->height - 1);
	x						 = _mm256_and_si256(x, widthMask);
	y						 = _mm256_and_si256(y, heightMask);

	const __m256i offset = _mm256_add_epi32(_mm256_mullo_epi32(y, _mm256_set1_epi32(pTex->width)), x);
	*pOut				 = _mm256_i32gather_epi32((const int*)pTex->pData, offset, 4);
}
#endif