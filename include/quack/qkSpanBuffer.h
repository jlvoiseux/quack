// include/quack/qkSpanBuffer.h
#pragma once

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
	qkSpan* spans;
	int*	spanCounts;
	int		height;
	int		maxSpansPerLine;
	int		totalCapacity;
} qkSpanBuffer;

int	 qkSpanBufferCreate(int height, int maxSpansPerLine, qkSpanBuffer* out);
void qkSpanBufferDestroy(qkSpanBuffer* buffer);
void qkSpanBufferClear(qkSpanBuffer* buffer);
void qkSpanBufferAddSpan(qkSpanBuffer* buffer, int y, int startX, int endX, float startZ, float endZ, float startU, float endU, float startV, float endV);