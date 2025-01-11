#include "quack/qkSpanBuffer.h"

#include <stdlib.h>
#include <string.h>

static int compareSpans(const void* a, const void* b)
{
	const qkSpan* spanA = (const qkSpan*)a;
	const qkSpan* spanB = (const qkSpan*)b;
	if (spanA->minZ < spanB->minZ)
		return -1;
	if (spanA->minZ > spanB->minZ)
		return 1;
	return 0;
}

int qkSpanBufferCreate(int height, int spansPerBlock, qkSpanBuffer* pOut)
{
	pOut->height		= height;
	pOut->spansPerBlock = spansPerBlock;
	pOut->totalBlocks	= 4;
	pOut->currentSpan	= 0;

	size_t totalSpans	   = pOut->totalBlocks * spansPerBlock;
	pOut->pSpans		   = malloc(totalSpans * sizeof(qkSpan));
	pOut->pScanlineCounts  = malloc(height * sizeof(uint16_t));
	pOut->pScanlineOffsets = malloc(height * sizeof(uint16_t));

	if (!pOut->pSpans || !pOut->pScanlineCounts || !pOut->pScanlineOffsets)
	{
		qkSpanBufferDestroy(pOut);
		return 0;
	}

	qkSpanBufferClear(pOut);
	return 1;
}

void qkSpanBufferDestroy(qkSpanBuffer* pBuffer)
{
	if (!pBuffer)
		return;

	free(pBuffer->pSpans);
	free(pBuffer->pScanlineCounts);
	free(pBuffer->pScanlineOffsets);
	memset(pBuffer, 0, sizeof(qkSpanBuffer));
}

void qkSpanBufferClear(qkSpanBuffer* pBuffer)
{
	memset(pBuffer->pScanlineCounts, 0, pBuffer->height * sizeof(uint16_t));
	pBuffer->currentSpan = 0;
}

static int qkSpanBufferGrow(qkSpanBuffer* pBuffer)
{
	int	   newTotalBlocks = pBuffer->totalBlocks * 2;
	size_t newSize		  = newTotalBlocks * pBuffer->spansPerBlock * sizeof(qkSpan);

	qkSpan* pNewSpans = realloc(pBuffer->pSpans, newSize);
	if (!pNewSpans)
		return 0;

	pBuffer->pSpans		 = pNewSpans;
	pBuffer->totalBlocks = newTotalBlocks;
	return 1;
}

void qkSpanBufferGenerate(qkSpanBuffer* pSpanBuffer, int y, float leftX, float rightX, float leftZ, float rightZ, float leftUOverZ, float rightUOverZ, float leftVOverZ, float rightVOverZ, float leftInvZ, float rightInvZ, bool perspective, int width, int height)
{
	if (leftX > rightX)
	{
		float temp;
		temp		= leftX;
		leftX		= rightX;
		rightX		= temp;
		temp		= leftZ;
		leftZ		= rightZ;
		rightZ		= temp;
		temp		= leftUOverZ;
		leftUOverZ	= rightUOverZ;
		rightUOverZ = temp;
		temp		= leftVOverZ;
		leftVOverZ	= rightVOverZ;
		rightVOverZ = temp;
		temp		= leftInvZ;
		leftInvZ	= rightInvZ;
		rightInvZ	= temp;
	}

	int startX = (int)fmaxf(0.0f, ceilf(leftX));
	int endX   = (int)fminf(width - 1, floorf(rightX));

	if (startX <= endX)
	{
		qkSpanBufferAdd(pSpanBuffer, y, startX, endX, leftZ, rightZ, leftUOverZ, rightUOverZ, leftVOverZ, rightVOverZ, leftInvZ, rightInvZ, perspective);
	}
}

void qkSpanBufferAdd(qkSpanBuffer* pBuffer, int y, int startX, int endX, float startZ, float endZ, float startUOverZ, float endUOverZ, float startVOverZ, float endVOverZ, float startInvZ, float endInvZ, bool perspective)
{
	if (y < 0 || y >= pBuffer->height)
		return;

	if (pBuffer->pScanlineCounts[y] == 0)
		pBuffer->pScanlineOffsets[y] = pBuffer->currentSpan;

	if (pBuffer->currentSpan >= pBuffer->totalBlocks * pBuffer->spansPerBlock)
	{
		if (!qkSpanBufferGrow(pBuffer))
			return;
	}

	qkSpan* pSpan	   = &pBuffer->pSpans[pBuffer->currentSpan];
	pSpan->startX	   = (int16_t)startX;
	pSpan->endX		   = (int16_t)endX;
	pSpan->startZ	   = startZ;
	pSpan->endZ		   = endZ;
	pSpan->startUOverZ = startUOverZ;
	pSpan->endUOverZ   = endUOverZ;
	pSpan->startVOverZ = startVOverZ;
	pSpan->endVOverZ   = endVOverZ;
	pSpan->startInvZ   = startInvZ;
	pSpan->endInvZ	   = endInvZ;
	pSpan->minZ		   = fminf(startZ, endZ);
	pSpan->perspective = perspective;

	pBuffer->pScanlineCounts[y]++;
	pBuffer->currentSpan++;
}

void qkSpanBufferProcess(qkSpanBuffer* pBuffer, int width, int height, uint32_t* pFrameBuffer, float* pZBuffer, const qkTexture* pTex)
{
	for (int y = 0; y < height; y++)
	{
		const int spanCount = pBuffer->pScanlineCounts[y];
		if (spanCount == 0)
			continue;

		const int baseOffset  = pBuffer->pScanlineOffsets[y];
		const int frameOffset = y * width;

		if (spanCount > 1)
		{
			qsort(&pBuffer->pSpans[baseOffset], spanCount, sizeof(qkSpan), compareSpans);
		}

		for (int i = 0; i < spanCount; i++)
		{
			const qkSpan* pSpan	 = &pBuffer->pSpans[baseOffset + i];
			const int	  startX = (int)pSpan->startX;
			const int	  endX	 = (int)pSpan->endX;

			if (startX < 0 || endX >= width || startX > endX)
				continue;

			const int	spanWidth	= endX - startX + 1;
			const float zStep		= (pSpan->endZ - pSpan->startZ) / spanWidth;
			float		z			= pSpan->startZ;
			int			pixelOffset = frameOffset + startX;

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
}