#pragma once

#include "qkTexture.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct qkSpan
{
	int16_t startX;
	int16_t endX;
	float	startZ;
	float	endZ;
	float	startUOverZ;
	float	endUOverZ;
	float	startVOverZ;
	float	endVOverZ;
	float	startInvZ;
	float	endInvZ;
	float	minZ;
	bool	perspective;
} qkSpan;

typedef struct qkSpanBuffer
{
	qkSpan*	  pSpans;
	uint16_t* pScanlineCounts;
	uint16_t* pScanlineOffsets;
	int		  height;
	int		  spansPerBlock;
	int		  totalBlocks;
	int		  currentSpan;
} qkSpanBuffer;

int	 qkSpanBufferCreate(int height, int spansPerBlock, qkSpanBuffer* pOut);
void qkSpanBufferDestroy(qkSpanBuffer* pBuffer);
void qkSpanBufferClear(qkSpanBuffer* pBuffer);
void qkSpanBufferGenerate(qkSpanBuffer* pSpanBuffer, int y, float leftX, float rightX, float leftZ, float rightZ, float leftUOverZ, float rightUOverZ, float leftVOverZ, float rightVOverZ, float leftInvZ, float rightInvZ, bool perspective, int width, int height);
void qkSpanBufferAdd(qkSpanBuffer* pBuffer, int y, int startX, int endX, float startZ, float endZ, float startUOverZ, float endUOverZ, float startVOverZ, float endVOverZ, float startInvZ, float endInvZ, bool perspective);
void qkSpanBufferProcess(qkSpanBuffer* pBuffer, int width, int height, uint32_t* pFrameBuffer, float* pZBuffer, const qkTexture* pTex);