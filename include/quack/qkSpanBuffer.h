#pragma once

#include "qkTexture.h"

#include <stdint.h>

typedef struct qkSpan
{
	int16_t startX;
	int16_t endX;
	float	startZ;
	float	endZ;
	float	startU;
	float	endU;
	float	startV;
	float	endV;
} qkSpan;

typedef struct qkSpanBuffer
{
	qkSpan*	  spans;
	uint16_t* scanlineCounts;
	uint16_t* scanlineOffsets;
	int		  height;
	int		  spansPerBlock;
	int		  totalBlocks;
	int		  currentSpan;
} qkSpanBuffer;

int	 qkSpanBufferCreate(int height, int spansPerBlock, qkSpanBuffer* pOut);
void qkSpanBufferDestroy(qkSpanBuffer* pBuffer);
void qkSpanBufferClear(qkSpanBuffer* pBuffer);
void qkSpanBufferAdd(qkSpanBuffer* pBuffer, int y, int startX, int endX, float startZ, float endZ, float startU, float endU, float startV, float endV);
void qkSpanBufferProcess(qkSpanBuffer* pBuffer, int width, int height, uint32_t* pFrameBuffer, float* pZBuffer, const qkTexture* pTex);