#include "quack/buffer/qkSpanBuffer.h"

#include <string.h>

int qkSpanBufferCreate(int height, size_t capacity, qkBuffer* pOut)
{
	if (!qkBufferCreate(capacity, pOut))
	{
		return 0;
	}

	pOut->pInt0[0] = height;
	return 1;
}

void qkSpanBufferDestroy(qkBuffer* pSpanBuffer)
{
	if (!pSpanBuffer)
		return;
	qkBufferDestroy(pSpanBuffer);
}

void qkSpanBufferClear(qkBuffer* pSpanBuffer)
{
	qkBufferClear(pSpanBuffer);
}

int qkSpanBufferAdd(qkBuffer* pSpanBuffer, int y, float startX, float endX, float startZ, float endZ, float startUOverZ, float endUOverZ, float startVOverZ, float endVOverZ, float startInvZ, float endInvZ, bool perspective, int width)
{
	if (y < 0 || y >= pSpanBuffer->pInt0[0] || qkBufferIsFull(pSpanBuffer))
	{
		return 0;
	}

	if (startX > endX)
	{
		float temp;
		temp   = startX;
		startX = endX;
		endX   = temp;

		temp   = startZ;
		startZ = endZ;
		endZ   = temp;

		temp		= startUOverZ;
		startUOverZ = endUOverZ;
		endUOverZ	= temp;

		temp		= startVOverZ;
		startVOverZ = endVOverZ;
		endVOverZ	= temp;

		temp	  = startInvZ;
		startInvZ = endInvZ;
		endInvZ	  = temp;
	}

	int startXBounded = (int)fmaxf(0.0f, ceilf(startX));
	int endXBounded	  = (int)fminf((float)width - 1, floorf(endX));

	if (startXBounded > endXBounded)
	{
		return 0;
	}

	size_t idx = pSpanBuffer->count;

	pSpanBuffer->pShort0[idx] = (int16_t)startXBounded;
	pSpanBuffer->pShort1[idx] = (int16_t)endXBounded;
	pSpanBuffer->pShort2[idx] = (int16_t)y;

	pSpanBuffer->pFloat0[idx] = startZ;
	pSpanBuffer->pFloat1[idx] = endZ;
	pSpanBuffer->pFloat2[idx] = fminf(startZ, endZ);

	pSpanBuffer->pFloat3[idx] = startUOverZ;
	pSpanBuffer->pFloat4[idx] = startVOverZ;
	pSpanBuffer->pFloat5[idx] = startInvZ;
	pSpanBuffer->pFloat6[idx] = endUOverZ;
	pSpanBuffer->pFloat7[idx] = endVOverZ;
	pSpanBuffer->pFloat8[idx] = endInvZ;

	pSpanBuffer->pInt1[idx] = perspective ? 1 : 0;
	pSpanBuffer->count++;

	return 1;
}

void qkSpanBufferProcess(qkBuffer* pSpanBuffer, int width, int height, uint32_t* pFrameBuffer, float* pZBuffer, const qkTexture* pTex)
{
	for (size_t i = 0; i < pSpanBuffer->count; i++)
	{
		const int startX = pSpanBuffer->pShort0[i];
		const int endX	 = pSpanBuffer->pShort1[i];
		const int y		 = pSpanBuffer->pShort2[i];

		if (startX < 0 || endX >= width || startX > endX || y < 0 || y >= height)
			continue;

		const int	spanWidth	= endX - startX + 1;
		const float startZ		= pSpanBuffer->pFloat0[i];
		const float endZ		= pSpanBuffer->pFloat1[i];
		const float zStep		= (endZ - startZ) / (float)spanWidth;
		float		z			= startZ;
		int			pixelOffset = y * width + startX;

		const float startUOverZ = pSpanBuffer->pFloat3[i];
		const float startVOverZ = pSpanBuffer->pFloat4[i];
		const float startInvZ	= pSpanBuffer->pFloat5[i];
		const float endUOverZ	= pSpanBuffer->pFloat6[i];
		const float endVOverZ	= pSpanBuffer->pFloat7[i];
		const float endInvZ		= pSpanBuffer->pFloat8[i];
		const bool	perspective = pSpanBuffer->pInt1[i] != 0;

		if (!perspective)
		{
			float u		= startUOverZ / startInvZ;
			float v		= startVOverZ / startInvZ;
			float uStep = ((endUOverZ / endInvZ) - u) / (float)spanWidth;
			float vStep = ((endVOverZ / endInvZ) - v) / (float)spanWidth;

			for (int x = 0; x < spanWidth; x++)
			{
				if (z > 0.0f && z < pZBuffer[pixelOffset])
				{
					pFrameBuffer[pixelOffset] = qkTextureSample(pTex, u, v);
					pZBuffer[pixelOffset]	  = z;
				}
				z += zStep;
				u += uStep;
				v += vStep;
				pixelOffset++;
			}
		}
		else
		{
			const float uOverZStep = (endUOverZ - startUOverZ) / (float)spanWidth;
			const float vOverZStep = (endVOverZ - startVOverZ) / (float)spanWidth;
			const float invZStep   = (endInvZ - startInvZ) / (float)spanWidth;

			float uOverZ = startUOverZ;
			float vOverZ = startVOverZ;
			float invZ	 = startInvZ;

			for (int x = 0; x < spanWidth; x++)
			{
				if (z > 0.0f && z < pZBuffer[pixelOffset])
				{
					const float invZRecip	  = 1.0f / invZ;
					float		u			  = uOverZ * invZRecip;
					float		v			  = vOverZ * invZRecip;
					pFrameBuffer[pixelOffset] = qkTextureSample(pTex, u, v);
					pZBuffer[pixelOffset]	  = z;
				}
				z += zStep;
				uOverZ += uOverZStep;
				vOverZ += vOverZStep;
				invZ += invZStep;
				pixelOffset++;
			}
		}
	}
}

#ifdef SIMD_ENABLE
void qkSpanBufferProcess8(qkBuffer* pSpanBuffer, int width, int height, uint32_t* pFrameBuffer, float* pZBuffer, const qkTexture* pTex)
{
	for (size_t i = 0; i < pSpanBuffer->count; i++)
	{
		const int startX = pSpanBuffer->pShort0[i];
		const int endX	 = pSpanBuffer->pShort1[i];
		const int y		 = pSpanBuffer->pShort2[i];

		if (startX < 0 || endX >= width || startX > endX || y < 0 || y >= height)
			continue;

		const int spanWidth	  = endX - startX + 1;
		const int pixelOffset = y * width + startX;

		const float startZ		= pSpanBuffer->pFloat0[i];
		const float zStep		= (pSpanBuffer->pFloat1[i] - startZ) / (float)spanWidth;
		const bool	perspective = pSpanBuffer->pInt1[i] != 0;

		__m256		 zVec	  = _mm256_setr_ps(startZ, startZ + zStep, startZ + zStep * 2, startZ + zStep * 3, startZ + zStep * 4, startZ + zStep * 5, startZ + zStep * 6, startZ + zStep * 7);
		const __m256 zStepVec = _mm256_set1_ps(zStep * 8.0f);

		__m256 uVec, vVec;
		__m256 uStepVec, vStepVec;
		__m256 uOverZVec, vOverZVec, invZVec;
		__m256 uOverZStepVec, vOverZStepVec, invZStepVec;

		const float u	  = !perspective ? pSpanBuffer->pFloat3[i] / pSpanBuffer->pFloat5[i] : 0.0f;
		const float v	  = !perspective ? pSpanBuffer->pFloat4[i] / pSpanBuffer->pFloat5[i] : 0.0f;
		const float uStep = !perspective ? ((pSpanBuffer->pFloat6[i] / pSpanBuffer->pFloat8[i]) - u) / (float)spanWidth : 0.0f;
		const float vStep = !perspective ? ((pSpanBuffer->pFloat7[i] / pSpanBuffer->pFloat8[i]) - v) / (float)spanWidth : 0.0f;

		const float uOverZStep = perspective ? (pSpanBuffer->pFloat6[i] - pSpanBuffer->pFloat3[i]) / (float)spanWidth : 0.0f;
		const float vOverZStep = perspective ? (pSpanBuffer->pFloat7[i] - pSpanBuffer->pFloat4[i]) / (float)spanWidth : 0.0f;
		const float invZStep   = perspective ? (pSpanBuffer->pFloat8[i] - pSpanBuffer->pFloat5[i]) / (float)spanWidth : 0.0f;

		if (!perspective)
		{
			uVec	 = _mm256_setr_ps(u, u + uStep, u + uStep * 2, u + uStep * 3, u + uStep * 4, u + uStep * 5, u + uStep * 6, u + uStep * 7);
			vVec	 = _mm256_setr_ps(v, v + vStep, v + vStep * 2, v + vStep * 3, v + vStep * 4, v + vStep * 5, v + vStep * 6, v + vStep * 7);
			uStepVec = _mm256_set1_ps(uStep * 8.0f);
			vStepVec = _mm256_set1_ps(vStep * 8.0f);
		}
		else
		{
			uOverZVec	  = _mm256_setr_ps(pSpanBuffer->pFloat3[i],
									   pSpanBuffer->pFloat3[i] + uOverZStep,
									   pSpanBuffer->pFloat3[i] + uOverZStep * 2,
									   pSpanBuffer->pFloat3[i] + uOverZStep * 3,
									   pSpanBuffer->pFloat3[i] + uOverZStep * 4,
									   pSpanBuffer->pFloat3[i] + uOverZStep * 5,
									   pSpanBuffer->pFloat3[i] + uOverZStep * 6,
									   pSpanBuffer->pFloat3[i] + uOverZStep * 7);
			vOverZVec	  = _mm256_setr_ps(pSpanBuffer->pFloat4[i],
									   pSpanBuffer->pFloat4[i] + vOverZStep,
									   pSpanBuffer->pFloat4[i] + vOverZStep * 2,
									   pSpanBuffer->pFloat4[i] + vOverZStep * 3,
									   pSpanBuffer->pFloat4[i] + vOverZStep * 4,
									   pSpanBuffer->pFloat4[i] + vOverZStep * 5,
									   pSpanBuffer->pFloat4[i] + vOverZStep * 6,
									   pSpanBuffer->pFloat4[i] + vOverZStep * 7);
			invZVec		  = _mm256_setr_ps(pSpanBuffer->pFloat5[i],
									   pSpanBuffer->pFloat5[i] + invZStep,
									   pSpanBuffer->pFloat5[i] + invZStep * 2,
									   pSpanBuffer->pFloat5[i] + invZStep * 3,
									   pSpanBuffer->pFloat5[i] + invZStep * 4,
									   pSpanBuffer->pFloat5[i] + invZStep * 5,
									   pSpanBuffer->pFloat5[i] + invZStep * 6,
									   pSpanBuffer->pFloat5[i] + invZStep * 7);
			uOverZStepVec = _mm256_set1_ps(uOverZStep * 8.0f);
			vOverZStepVec = _mm256_set1_ps(vOverZStep * 8.0f);
			invZStepVec	  = _mm256_set1_ps(invZStep * 8.0f);
		}

		for (int x = 0; x < spanWidth; x += 8)
		{
			const int remainingPixels = spanWidth - x;
			if (remainingPixels < 8)
				break;

			const __m256 zBuffer = _mm256_load_ps(&pZBuffer[pixelOffset + x]);
			const __m256 mask	 = _mm256_and_ps(_mm256_cmp_ps(zVec, _mm256_set1_ps(0.0f), _CMP_GT_OQ), _mm256_cmp_ps(zVec, zBuffer, _CMP_LT_OQ));

			if (_mm256_movemask_ps(mask))
			{
				__m256i colors;
				if (!perspective)
				{
					qkTextureSample8(pTex, uVec, vVec, &colors);
				}
				else
				{
					const __m256 invZRecip = _mm256_rcp_ps(invZVec);
					const __m256 uVec	   = _mm256_mul_ps(uOverZVec, invZRecip);
					const __m256 vVec	   = _mm256_mul_ps(vOverZVec, invZRecip);
					qkTextureSample8(pTex, uVec, vVec, &colors);
				}

				__m256i* pDest	  = (__m256i*)&pFrameBuffer[pixelOffset + x];
				__m256i	 existing = _mm256_load_si256(pDest);
				__m256i	 masked	  = _mm256_blendv_epi8(existing, colors, _mm256_castps_si256(mask));
				_mm256_store_si256(pDest, masked);
				_mm256_maskstore_ps(&pZBuffer[pixelOffset + x], _mm256_castps_si256(mask), zVec);
			}

			zVec = _mm256_add_ps(zVec, zStepVec);
			if (!perspective)
			{
				uVec = _mm256_add_ps(uVec, uStepVec);
				vVec = _mm256_add_ps(vVec, vStepVec);
			}
			else
			{
				uOverZVec = _mm256_add_ps(uOverZVec, uOverZStepVec);
				vOverZVec = _mm256_add_ps(vOverZVec, vOverZStepVec);
				invZVec	  = _mm256_add_ps(invZVec, invZStepVec);
			}
		}

		const int remainingStart = (spanWidth / 8) * 8;
		if (remainingStart < spanWidth)
		{
			float z = startZ + (float)remainingStart * zStep;
			if (!perspective)
			{
				float currU = u + uStep * (float)remainingStart;
				float currV = v + vStep * (float)remainingStart;

				for (int x = remainingStart; x < spanWidth; x++)
				{
					if (z > 0.0f && z < pZBuffer[pixelOffset + x])
					{
						pFrameBuffer[pixelOffset + x] = qkTextureSample(pTex, currU, currV);
						pZBuffer[pixelOffset + x]	  = z;
					}
					z += zStep;
					currU += uStep;
					currV += vStep;
				}
			}
			else
			{
				float uOverZ = pSpanBuffer->pFloat3[i] + uOverZStep * (float)remainingStart;
				float vOverZ = pSpanBuffer->pFloat4[i] + vOverZStep * (float)remainingStart;
				float invZ	 = pSpanBuffer->pFloat5[i] + invZStep * (float)remainingStart;

				for (int x = remainingStart; x < spanWidth; x++)
				{
					if (z > 0.0f && z < pZBuffer[pixelOffset + x])
					{
						const float invZRecip		  = 1.0f / invZ;
						const float invU			  = uOverZ * invZRecip;
						const float invV			  = vOverZ * invZRecip;
						pFrameBuffer[pixelOffset + x] = qkTextureSample(pTex, invU, invV);
						pZBuffer[pixelOffset + x]	  = z;
					}
					z += zStep;
					uOverZ += uOverZStep;
					vOverZ += vOverZStep;
					invZ += invZStep;
				}
			}
		}
	}
}
#endif