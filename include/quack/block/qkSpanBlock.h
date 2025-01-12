#pragma once

#include "quack/block/qkBlock.h"
#include "quack/qkTexture.h"

#include <stdint.h>

int	 qkSpanBlockCreate(int height, size_t capacity, qkBlock* pOut);
void qkSpanBlockDestroy(qkBlock* pSpanBlock);
void qkSpanBlockClear(qkBlock* pSpanBlock);
int	 qkSpanBlockAdd(qkBlock* pSpanBlock, int y, float startX, float endX, float startZ, float endZ, float startUOverZ, float endUOverZ, float startVOverZ, float endVOverZ, float startInvZ, float endInvZ, bool perspective, int width);
void qkSpanBlockProcess(qkBlock* pSpanBlock, int width, int height, uint32_t* pFrameBuffer, float* pZBuffer, const qkTexture* pTex);