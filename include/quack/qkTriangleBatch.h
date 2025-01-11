#pragma once

#include "quack/math/qkVec3.h"
#include "quack/qkSpanBuffer.h"
#include "quack/qkTexture.h"

#define QK_BATCH_SIZE 64

typedef struct qkTriangleBatch
{
	float* pPositions;
	float* pTexCoords;
	float* pInvDepths;
	int*   pIndices;
	int	   count;
} qkTriangleBatch;

int	 qkTriangleBatchCreate(qkTriangleBatch* pBatch);
void qkTriangleBatchDestroy(qkTriangleBatch* pBatch);
void qkTriangleBatchClear(qkTriangleBatch* pBatch);
int	 qkTriangleBatchAdd(qkTriangleBatch* pBatch, const qkVec3* pPos1, const qkVec3* pPos2, const qkVec3* pPos3, float u1, float v1, float u2, float v2, float u3, float v3);
void qkTriangleBatchProcess(qkTriangleBatch* pBatch, qkSpanBuffer* pSpanBuffer, int width, int height, uint32_t* pFrameBuffer, float* pZBuffer, const qkTexture* pTex);