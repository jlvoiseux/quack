#include "quack/qkSpanBuffer.h"

#include <stdlib.h>
#include <string.h>

int qkSpanBufferCreate(int height, size_t spansPerBlock, size_t blockCount, qkSpanBuffer* pOut)
{
	pOut->pBlocks = malloc(sizeof(qkBlock) * blockCount);
	if (!pOut->pBlocks)
	{
		return 0;
	}

	for (size_t i = 0; i < blockCount; i++)
	{
		if (!qkBlockCreate(sizeof(qkSpan), spansPerBlock, &pOut->pBlocks[i]))
		{
			for (size_t j = 0; j < i; j++)
			{
				qkBlockDestroy(&pOut->pBlocks[j]);
			}
			free(pOut->pBlocks);
			return 0;
		}
	}

	pOut->pSpanCounts  = malloc(height * sizeof(uint16_t));
	pOut->pSpanOffsets = malloc(height * sizeof(uint16_t));

	if (!pOut->pSpanCounts || !pOut->pSpanOffsets)
	{
		qkSpanBufferDestroy(pOut);
		return 0;
	}

	pOut->blockCount  = blockCount;
	pOut->activeBlock = 0;
	pOut->height	  = height;

	qkSpanBufferClear(pOut);
	return 1;
}

void qkSpanBufferDestroy(qkSpanBuffer* pBuffer)
{
	if (!pBuffer)
		return;

	for (size_t i = 0; i < pBuffer->blockCount; i++)
	{
		qkBlockDestroy(&pBuffer->pBlocks[i]);
	}

	free(pBuffer->pBlocks);
	free(pBuffer->pSpanCounts);
	free(pBuffer->pSpanOffsets);
	memset(pBuffer, 0, sizeof(qkSpanBuffer));
}

void qkSpanBufferClear(qkSpanBuffer* pBuffer)
{
	memset(pBuffer->pSpanCounts, 0, pBuffer->height * sizeof(uint16_t));
	for (size_t i = 0; i < pBuffer->blockCount; i++)
	{
		qkBlockClear(&pBuffer->pBlocks[i]);
	}
	pBuffer->activeBlock = 0;
}

void qkSpanBufferAdd(qkSpanBuffer* pBuffer, int y, int startX, int endX, float startZ, float endZ, float startUOverZ, float endUOverZ, float startVOverZ, float endVOverZ, float startInvZ, float endInvZ, bool perspective)
{
	if (y < 0 || y >= pBuffer->height)
		return;

	qkSpan span = {.startX		= (int16_t)startX,
				   .endX		= (int16_t)endX,
				   .y			= (int16_t)y,
				   .startZ		= startZ,
				   .endZ		= endZ,
				   .startUOverZ = startUOverZ,
				   .endUOverZ	= endUOverZ,
				   .startVOverZ = startVOverZ,
				   .endVOverZ	= endVOverZ,
				   .startInvZ	= startInvZ,
				   .endInvZ		= endInvZ,
				   .minZ		= startZ < endZ ? startZ : endZ,
				   .perspective = perspective};

	if (qkBlockIsFull(&pBuffer->pBlocks[pBuffer->activeBlock]))
	{
		if (pBuffer->activeBlock < pBuffer->blockCount - 1)
		{
			pBuffer->activeBlock++;
		}
		else
		{
			size_t	 newBlockCount = pBuffer->blockCount * 2;
			qkBlock* pNewBlocks	   = realloc(pBuffer->pBlocks, sizeof(qkBlock) * newBlockCount);
			if (!pNewBlocks)
			{
				qkSpanBufferProcess(pBuffer, 0, 0, NULL, NULL, NULL);
				qkSpanBufferClear(pBuffer);
			}
			else
			{
				for (size_t i = pBuffer->blockCount; i < newBlockCount; i++)
				{
					if (!qkBlockCreate(sizeof(qkSpan), pBuffer->pBlocks[0].capacity, &pNewBlocks[i]))
					{
						newBlockCount = i;
						break;
					}
				}
				pBuffer->pBlocks	= pNewBlocks;
				pBuffer->blockCount = newBlockCount;
				pBuffer->activeBlock++;
			}
		}
	}

	qkBlockAdd(&pBuffer->pBlocks[pBuffer->activeBlock], &span);
	pBuffer->pSpanCounts[y]++;
}

static void processBlock(qkBlock* pBlock, int width, int height, uint32_t* pFrameBuffer, float* pZBuffer, const qkTexture* pTex)
{
	for (size_t i = 0; i < pBlock->count; i++)
	{
		const qkSpan* pSpan	 = qkBlockGetElement(pBlock, i);
		const int	  startX = (int)pSpan->startX;
		const int	  endX	 = (int)pSpan->endX;
		const int	  y		 = pSpan->y;

		if (startX < 0 || endX >= width || startX > endX || y < 0 || y >= height)
		{
			continue;
		}

		const int	spanWidth	= endX - startX + 1;
		const float zStep		= (pSpan->endZ - pSpan->startZ) / spanWidth;
		float		z			= pSpan->startZ;
		int			pixelOffset = y * width + startX;

		if (!pSpan->perspective)
		{
			float u		= pSpan->startUOverZ / pSpan->startInvZ;
			float v		= pSpan->startVOverZ / pSpan->startInvZ;
			float uStep = ((pSpan->endUOverZ / pSpan->endInvZ) - u) / spanWidth;
			float vStep = ((pSpan->endVOverZ / pSpan->endInvZ) - v) / spanWidth;

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
			const float uOverZStep = (pSpan->endUOverZ - pSpan->startUOverZ) / spanWidth;
			const float vOverZStep = (pSpan->endVOverZ - pSpan->startVOverZ) / spanWidth;
			const float invZStep   = (pSpan->endInvZ - pSpan->startInvZ) / spanWidth;

			float uOverZ = pSpan->startUOverZ;
			float vOverZ = pSpan->startVOverZ;
			float invZ	 = pSpan->startInvZ;

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

void qkSpanBufferProcess(qkSpanBuffer* pBuffer, int width, int height, uint32_t* pFrameBuffer, float* pZBuffer, const qkTexture* pTex)
{
	// Process all active blocks
	for (size_t i = 0; i <= pBuffer->activeBlock; i++)
	{
		if (pBuffer->pBlocks[i].count > 0)
		{
			processBlock(&pBuffer->pBlocks[i], width, height, pFrameBuffer, pZBuffer, pTex);
		}
	}
}

void qkSpanBufferGenerate(qkSpanBuffer* pSpanBuffer, int y, float leftX, float rightX, float leftZ, float rightZ, float leftUOverZ, float rightUOverZ, float leftVOverZ, float rightVOverZ, float leftInvZ, float rightInvZ, bool perspective, int width, int height)
{
	if (leftX > rightX)
	{
		float temp;
		temp   = leftX;
		leftX  = rightX;
		rightX = temp;

		temp   = leftZ;
		leftZ  = rightZ;
		rightZ = temp;

		temp		= leftUOverZ;
		leftUOverZ	= rightUOverZ;
		rightUOverZ = temp;

		temp		= leftVOverZ;
		leftVOverZ	= rightVOverZ;
		rightVOverZ = temp;

		temp	  = leftInvZ;
		leftInvZ  = rightInvZ;
		rightInvZ = temp;
	}

	int startX = (int)fmaxf(0.0f, ceilf(leftX));
	int endX   = (int)fminf(width - 1, floorf(rightX));

	if (startX <= endX)
	{
		qkSpanBufferAdd(pSpanBuffer, y, startX, endX, leftZ, rightZ, leftUOverZ, rightUOverZ, leftVOverZ, rightVOverZ, leftInvZ, rightInvZ, perspective);
	}
}