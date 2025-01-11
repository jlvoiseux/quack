// include/quack/qkSpanBuffer.h
#pragma once

#include "qkTexture.h"

#include <stdint.h>

typedef struct qkSpan
{
	int	  startX;
	int	  endX;
	float startZ;
	float endZ;
	float startU;
	float endU;
	float startV;
	float endV;
} qkSpan;

typedef struct qkSpanBuffer
{
	qkSpan* pSpans;
	int*	pSpanCounts;
	int		height;
	int		maxSpansPerLine;
	int		totalCapacity;
} qkSpanBuffer;

int	 qkSpanBufferCreate(int height, int maxSpansPerLine, qkSpanBuffer* pOut);
void qkSpanBufferDestroy(qkSpanBuffer* pBuffer);
void qkSpanBufferClear(qkSpanBuffer* pBuffer);
void qkSpanBufferAdd(qkSpanBuffer* pBuffer, int y, int startX, int endX, float startZ, float endZ, float startU, float endU, float startV, float endV);
void qkSpanBufferProcess(qkSpanBuffer* pBuffer, int width, int height, uint32_t* pFrameBuffer, float* pZBuffer, const qkTexture* pTex);