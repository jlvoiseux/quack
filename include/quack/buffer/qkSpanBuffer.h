#pragma once

#include "quack/buffer/qkBuffer.h"
#include "quack/qkTexture.h"
#ifdef SIMD_ENABLE
#include <immintrin.h>
#endif
#include <stdint.h>

int	 qkSpanBufferCreate(int height, size_t capacity, qkBuffer* pOut);
void qkSpanBufferDestroy(qkBuffer* pSpanBuffer);
void qkSpanBufferClear(qkBuffer* pSpanBuffer);
int	 qkSpanBufferAdd(qkBuffer* pSpanBuffer, int y, float startX, float endX, float startZ, float endZ, float startUOverZ, float endUOverZ, float startVOverZ, float endVOverZ, float startInvZ, float endInvZ, bool perspective, int width);
void qkSpanBufferProcess(qkBuffer* pSpanBuffer, int width, int height, uint32_t* pFrameBuffer, float* pZBuffer, const qkTexture* pTex);
#ifdef SIMD_ENABLE
void qkSpanBufferProcess8(qkBuffer* pSpanBuffer, int width, int height, uint32_t* pFrameBuffer, float* pZBuffer, const qkTexture* pTex);
#endif