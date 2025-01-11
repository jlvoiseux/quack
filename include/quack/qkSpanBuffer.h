// qkSpanBuffer.h
#pragma once

#include "qkBlock.h"
#include "qkTexture.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct qkSpan
{
	int16_t startX;
	int16_t endX;
	int16_t y;
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
	qkBlock*  pBlocks;
	size_t	  blockCount;
	size_t	  activeBlock;
	uint16_t* pSpanCounts;
	uint16_t* pSpanOffsets;
	int		  height;
} qkSpanBuffer;

int	 qkSpanBufferCreate(int height, size_t spansPerBlock, size_t blockCount, qkSpanBuffer* pOut);
void qkSpanBufferDestroy(qkSpanBuffer* pBuffer);
void qkSpanBufferClear(qkSpanBuffer* pBuffer);
void qkSpanBufferGenerate(qkSpanBuffer* pSpanBuffer, int y, float leftX, float rightX, float leftZ, float rightZ, float leftUOverZ, float rightUOverZ, float leftVOverZ, float rightVOverZ, float leftInvZ, float rightInvZ, bool perspective, int width, int height);
void qkSpanBufferAdd(qkSpanBuffer* pBuffer, int y, int startX, int endX, float startZ, float endZ, float startUOverZ, float endUOverZ, float startVOverZ, float endVOverZ, float startInvZ, float endInvZ, bool perspective);
void qkSpanBufferProcess(qkSpanBuffer* pBuffer, int width, int height, uint32_t* pFrameBuffer, float* pZBuffer, const qkTexture* pTex);