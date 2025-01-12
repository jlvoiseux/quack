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