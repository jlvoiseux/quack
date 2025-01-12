#pragma once

#include "quack/block/qkSpanBlock.h"
#include "quack/math/qkVec3.h"
#include "quack/qkTexture.h"

int	 qkTriangleBlockCreate(size_t capacity, qkBlock* pTriangleBlock);
void qkTriangleBlockDestroy(qkBlock* pTriangleBlock);
void qkTriangleBlockClear(qkBlock* pTriangleBlock);
int	 qkTriangleBlockAdd(qkBlock* pTriangleBlock, const qkVec3* pPos1, const qkVec3* pPos2, const qkVec3* pPos3, float u1, float v1, float u2, float v2, float u3, float v3);
void qkTriangleBlockProcess(qkBlock* pTriangleBlock, qkBlock* pSpanBlock, int width, int height, uint32_t* pFrameBuffer, float* pZBuffer, const qkTexture* pTex);