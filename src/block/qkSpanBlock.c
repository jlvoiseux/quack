#include "quack/block/qkSpanBlock.h"

#include <string.h>

int qkSpanBlockCreate(int height, size_t capacity, qkBlock* pOut)
{
	if (!qkBlockCreate(capacity, pOut))
	{
		return 0;
	}

	pOut->pInt0[0] = height;
	return 1;
}

void qkSpanBlockDestroy(qkBlock* pSpanBlock)
{
	if (!pSpanBlock)
		return;
	qkBlockDestroy(pSpanBlock);
}

void qkSpanBlockClear(qkBlock* pSpanBlock)
{
	qkBlockClear(pSpanBlock);
}

int qkSpanBlockAdd(qkBlock* pSpanBlock, int y, float startX, float endX, float startZ, float endZ, float startUOverZ, float endUOverZ, float startVOverZ, float endVOverZ, float startInvZ, float endInvZ, bool perspective, int width)
{
	if (y < 0 || y >= pSpanBlock->pInt0[0] || qkBlockIsFull(pSpanBlock))
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

	size_t idx = pSpanBlock->count;

	pSpanBlock->pShort0[idx] = (int16_t)startXBounded;
	pSpanBlock->pShort1[idx] = (int16_t)endXBounded;
	pSpanBlock->pShort2[idx] = (int16_t)y;

	pSpanBlock->pFloat0[idx] = startZ;
	pSpanBlock->pFloat1[idx] = endZ;
	pSpanBlock->pFloat2[idx] = fminf(startZ, endZ);

	pSpanBlock->pFloat3[idx] = startUOverZ;
	pSpanBlock->pFloat4[idx] = startVOverZ;
	pSpanBlock->pFloat5[idx] = startInvZ;
	pSpanBlock->pFloat6[idx] = endUOverZ;
	pSpanBlock->pFloat7[idx] = endVOverZ;
	pSpanBlock->pFloat8[idx] = endInvZ;

	pSpanBlock->pInt1[idx] = perspective ? 1 : 0;
	pSpanBlock->count++;

	return 1;
}

void qkSpanBlockProcess(qkBlock* pSpanBlock, int width, int height, uint32_t* pFrameBuffer, float* pZBuffer, const qkTexture* pTex)
{
	for (size_t i = 0; i < pSpanBlock->count; i++)
	{
		const int startX = pSpanBlock->pShort0[i];
		const int endX	 = pSpanBlock->pShort1[i];
		const int y		 = pSpanBlock->pShort2[i];

		if (startX < 0 || endX >= width || startX > endX || y < 0 || y >= height)
			continue;

		const int	spanWidth	= endX - startX + 1;
		const float startZ		= pSpanBlock->pFloat0[i];
		const float endZ		= pSpanBlock->pFloat1[i];
		const float zStep		= (endZ - startZ) / (float)spanWidth;
		float		z			= startZ;
		int			pixelOffset = y * width + startX;

		const float startUOverZ = pSpanBlock->pFloat3[i];
		const float startVOverZ = pSpanBlock->pFloat4[i];
		const float startInvZ	= pSpanBlock->pFloat5[i];
		const float endUOverZ	= pSpanBlock->pFloat6[i];
		const float endVOverZ	= pSpanBlock->pFloat7[i];
		const float endInvZ		= pSpanBlock->pFloat8[i];
		const bool	perspective = pSpanBlock->pInt1[i] != 0;

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