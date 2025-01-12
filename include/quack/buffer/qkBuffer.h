#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define QK_BLOCK_ALIGN 32

typedef struct qkBuffer
{
	float* pFloat0;
	float* pFloat1;
	float* pFloat2;
	float* pFloat3;
	float* pFloat4;
	float* pFloat5;
	float* pFloat6;
	float* pFloat7;
	float* pFloat8;

	int32_t* pInt0;
	int32_t* pInt1;

	int16_t* pShort0;
	int16_t* pShort1;
	int16_t* pShort2;

	size_t capacity;
	size_t count;
} qkBuffer;

void* qkBufferAlignedAlloc(size_t size);
void  qkBufferAlignedFree(void* ptr);

int	 qkBufferCreate(size_t capacity, qkBuffer* pOut);
void qkBufferDestroy(qkBuffer* pBuffer);
void qkBufferClear(qkBuffer* pBuffer);
bool qkBufferIsFull(const qkBuffer* pBuffer);