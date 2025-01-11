#include "quack/qkSpanBuffer.h"

#include <stdlib.h>
#include <string.h>

int qkSpanBufferCreate(int height, int spansPerBlock, qkSpanBuffer* pOut)
{
	pOut->height		= height;
	pOut->spansPerBlock = spansPerBlock;
	pOut->totalBlocks	= 4;
	pOut->currentSpan	= 0;

	size_t totalSpans	  = pOut->totalBlocks * spansPerBlock;
	pOut->spans			  = malloc(totalSpans * sizeof(qkSpan));
	pOut->scanlineCounts  = malloc(height * sizeof(uint16_t));
	pOut->scanlineOffsets = malloc(height * sizeof(uint16_t));

	if (!pOut->spans || !pOut->scanlineCounts || !pOut->scanlineOffsets)
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
	{
		return;
	}

	free(pBuffer->spans);
	free(pBuffer->scanlineCounts);
	free(pBuffer->scanlineOffsets);
	memset(pBuffer, 0, sizeof(qkSpanBuffer));
}

void qkSpanBufferClear(qkSpanBuffer* pBuffer)
{
	memset(pBuffer->scanlineCounts, 0, pBuffer->height * sizeof(uint16_t));
	pBuffer->currentSpan = 0;
}

static int qkSpanBufferGrow(qkSpanBuffer* pBuffer)
{
	int	   newTotalBlocks = pBuffer->totalBlocks * 2;
	size_t newSize		  = newTotalBlocks * pBuffer->spansPerBlock * sizeof(qkSpan);

	qkSpan* pNewSpans = realloc(pBuffer->spans, newSize);
	if (!pNewSpans)
	{
		return 0;
	}

	pBuffer->spans		 = pNewSpans;
	pBuffer->totalBlocks = newTotalBlocks;
	return 1;
}

void qkSpanBufferGenerate(qkSpanBuffer* pSpanBuffer, int y, float leftX, float rightX, float leftZ, float rightZ, float leftU, float rightU, float leftV, float rightV, int width, int height)
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
		temp   = leftU;
		leftU  = rightU;
		rightU = temp;
		temp   = leftV;
		leftV  = rightV;
		rightV = temp;
	}

	int startX = (int)fmaxf(0.0f, ceilf(leftX));
	int endX   = (int)fminf(width - 1, floorf(rightX));

	if (startX <= endX)
	{
		qkSpanBufferAdd(pSpanBuffer, y, startX, endX, leftZ, rightZ, leftU, rightU, leftV, rightV);
	}
}

void qkSpanBufferAdd(qkSpanBuffer* pBuffer, int y, int startX, int endX, float startZ, float endZ, float startU, float endU, float startV, float endV)
{
	if (y < 0 || y >= pBuffer->height)
	{
		return;
	}

	// Store the offset for this scanline if it's the first span
	if (pBuffer->scanlineCounts[y] == 0)
	{
		pBuffer->scanlineOffsets[y] = pBuffer->currentSpan;
	}

	// Check if we need to grow
	if (pBuffer->currentSpan >= pBuffer->totalBlocks * pBuffer->spansPerBlock)
	{
		if (!qkSpanBufferGrow(pBuffer))
		{
			return;
		}
	}

	qkSpan* pSpan = &pBuffer->spans[pBuffer->currentSpan];
	pSpan->startX = (int16_t)startX;
	pSpan->endX	  = (int16_t)endX;
	pSpan->startZ = startZ;
	pSpan->endZ	  = endZ;
	pSpan->startU = startU;
	pSpan->endU	  = endU;
	pSpan->startV = startV;
	pSpan->endV	  = endV;

	pBuffer->scanlineCounts[y]++;
	pBuffer->currentSpan++;
}

void qkSpanBufferProcess(qkSpanBuffer* pBuffer, int width, int height, uint32_t* pFrameBuffer, float* pZBuffer, const qkTexture* pTex)
{
	for (int y = 0; y < height; y++)
	{
		const int spanCount = pBuffer->scanlineCounts[y];
		if (spanCount == 0)
		{
			continue;
		}

		const int baseOffset  = pBuffer->scanlineOffsets[y];
		const int frameOffset = y * width;

		for (int i = 0; i < spanCount; i++)
		{
			const qkSpan* pSpan	 = &pBuffer->spans[baseOffset + i];
			const int	  startX = (int)pSpan->startX;
			const int	  endX	 = (int)pSpan->endX;

			if (startX < 0 || endX >= width || startX > endX)
			{
				continue;
			}

			const int	spanWidth = endX - startX + 1;
			const float zStep	  = (pSpan->endZ - pSpan->startZ) / spanWidth;
			const float uStep	  = (pSpan->endU - pSpan->startU) / spanWidth;
			const float vStep	  = (pSpan->endV - pSpan->startV) / spanWidth;

			float z = pSpan->startZ;
			float u = pSpan->startU;
			float v = pSpan->startV;

			int pixelOffset = frameOffset + startX;

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
	}
}