#pragma once

#include "quack/buffer/qkSpanBuffer.h"
#include "quack/math/qkVec3.h"
#include "quack/qkTexture.h"

int	 qkVertexBufferCreate(size_t capacity, qkBuffer* pVertexBuffer);
void qkVertexBufferDestroy(qkBuffer* pVertexBuffer);
void qkVertexBufferClear(qkBuffer* pVertexBuffer);
int	 qkVertexBufferAdd(qkBuffer* pVertexBuffer, const qkVec3* pPos, float u, float v);
void qkVertexProcess(qkBuffer* pVertexBuffer0, qkBuffer* pVertexBuffer1, qkBuffer* pVertexBuffer2, qkBuffer* pSpanBuffer, int width, int height, uint32_t* pFrameBuffer, float* pZBuffer, const qkTexture* pTex);