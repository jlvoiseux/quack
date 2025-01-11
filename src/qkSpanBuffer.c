// src/qkSpanBuffer.c
#include "quack/qkSpanBuffer.h"

#include <stdlib.h>
#include <string.h>

int qkSpanBufferCreate(int height, int maxSpansPerLine, qkSpanBuffer* pOut)
{
	pOut->height		  = height;
	pOut->maxSpansPerLine = maxSpansPerLine;
	pOut->totalCapacity	  = height * maxSpansPerLine;

	pOut->pSpans = malloc(sizeof(qkSpan) * pOut->totalCapacity);
	if (!pOut->pSpans)
	{
		return -1;
	}

	pOut->pSpanCounts = malloc(sizeof(int) * height);
	if (!pOut->pSpanCounts)
	{
		free(pOut->pSpans);
		return -2;
	}

	qkSpanBufferClear(pOut);
	return 0;
}

void qkSpanBufferDestroy(qkSpanBuffer* pBuffer)
{
	if (!pBuffer)
	{
		return;
	}

	free(pBuffer->pSpans);
	free(pBuffer->pSpanCounts);
	memset(pBuffer, 0, sizeof(qkSpanBuffer));
}

void qkSpanBufferClear(qkSpanBuffer* pBuffer)
{
	memset(pBuffer->pSpanCounts, 0, sizeof(int) * pBuffer->height);
}

void qkSpanBufferAdd(qkSpanBuffer* pBuffer, int y, int startX, int endX, float startZ, float endZ, float startU, float endU, float startV, float endV)
{
	if (y < 0 || y >= pBuffer->height || pBuffer->pSpanCounts[y] >= pBuffer->maxSpansPerLine)
	{
		return;
	}

	int		spanIdx = y * pBuffer->maxSpansPerLine + pBuffer->pSpanCounts[y];
	qkSpan* span	= &pBuffer->pSpans[spanIdx];

	span->startX = startX;
	span->endX	 = endX;
	span->startZ = startZ;
	span->endZ	 = endZ;
	span->startU = startU;
	span->endU	 = endU;
	span->startV = startV;
	span->endV	 = endV;

	pBuffer->pSpanCounts[y]++;
}

void qkSpanBufferProcess(qkSpanBuffer* pBuffer, int width, int height, uint32_t* pFrameBuffer, float* pZBuffer, const qkTexture* pTex)
{
	for (int y = 0; y < height; y++)
	{
		int		spanCount = pBuffer->pSpanCounts[y];
		qkSpan* spans	  = &pBuffer->pSpans[y * pBuffer->maxSpansPerLine];

		for (int i = 0; i < spanCount; i++)
		{
			qkSpan* span	  = &spans[i];
			int		spanWidth = span->endX - span->startX + 1;
			if (spanWidth <= 0)
				continue;

			float zStep = (span->endZ - span->startZ) / spanWidth;
			float uStep = (span->endU - span->startU) / spanWidth;
			float vStep = (span->endV - span->startV) / spanWidth;

			float z = span->startZ;
			float u = span->startU;
			float v = span->startV;

			int offset = y * width + span->startX;
			for (int x = 0; x < spanWidth; x++)
			{
				if (z > 0.0f && z < pZBuffer[offset])
				{
					pFrameBuffer[offset] = qkTextureSample(pTex, u, v);
					pZBuffer[offset]	 = z;
				}
				z += zStep;
				u += uStep;
				v += vStep;
				offset++;
			}
		}
	}
}