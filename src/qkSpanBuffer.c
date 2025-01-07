// src/qkSpanBuffer.c
#include "quack/qkSpanBuffer.h"

#include <stdlib.h>
#include <string.h>

int qkSpanBufferCreate(int height, int maxSpansPerLine, qkSpanBuffer* out)
{
	out->height			 = height;
	out->maxSpansPerLine = maxSpansPerLine;
	out->totalCapacity	 = height * maxSpansPerLine;

	out->spans = malloc(sizeof(qkSpan) * out->totalCapacity);
	if (!out->spans)
	{
		return -1;
	}

	out->spanCounts = malloc(sizeof(int) * height);
	if (!out->spanCounts)
	{
		free(out->spans);
		return -2;
	}

	qkSpanBufferClear(out);
	return 0;
}

void qkSpanBufferDestroy(qkSpanBuffer* buffer)
{
	if (!buffer)
	{
		return;
	}

	free(buffer->spans);
	free(buffer->spanCounts);
	memset(buffer, 0, sizeof(qkSpanBuffer));
}

void qkSpanBufferClear(qkSpanBuffer* buffer)
{
	memset(buffer->spanCounts, 0, sizeof(int) * buffer->height);
}

void qkSpanBufferAddSpan(qkSpanBuffer* buffer, int y, int startX, int endX, float startZ, float endZ, float startU, float endU, float startV, float endV)
{
	if (y < 0 || y >= buffer->height || buffer->spanCounts[y] >= buffer->maxSpansPerLine)
	{
		return;
	}

	int		spanIdx = y * buffer->maxSpansPerLine + buffer->spanCounts[y];
	qkSpan* span	= &buffer->spans[spanIdx];

	span->startX = startX;
	span->endX	 = endX;
	span->startZ = startZ;
	span->endZ	 = endZ;
	span->startU = startU;
	span->endU	 = endU;
	span->startV = startV;
	span->endV	 = endV;

	buffer->spanCounts[y]++;
}