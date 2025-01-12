#include "quack/buffer/qkBuffer.h"

#include <stdlib.h>
#include <string.h>

void* qkBufferAlignedAlloc(size_t size)
{
	void* ptr = NULL;
#ifdef _WIN32
	ptr = _aligned_malloc(size, QK_BLOCK_ALIGN);
#else
	if (posix_memalign(&ptr, QK_BLOCK_ALIGN, size) != 0)
	{
		ptr = NULL;
	}
#endif
	return ptr;
}

void qkBufferAlignedFree(void* ptr)
{
#ifdef _WIN32
	_aligned_free(ptr);
#else
	free(ptr);
#endif
}

int qkBufferCreate(size_t capacity, qkBuffer* pOut)
{
	pOut->pFloat0 = qkBufferAlignedAlloc(capacity * sizeof(float));
	pOut->pFloat1 = qkBufferAlignedAlloc(capacity * sizeof(float));
	pOut->pFloat2 = qkBufferAlignedAlloc(capacity * sizeof(float));
	pOut->pFloat3 = qkBufferAlignedAlloc(capacity * sizeof(float));
	pOut->pFloat4 = qkBufferAlignedAlloc(capacity * sizeof(float));
	pOut->pFloat5 = qkBufferAlignedAlloc(capacity * sizeof(float));
	pOut->pFloat6 = qkBufferAlignedAlloc(capacity * sizeof(float));
	pOut->pFloat7 = qkBufferAlignedAlloc(capacity * sizeof(float));
	pOut->pFloat8 = qkBufferAlignedAlloc(capacity * sizeof(float));

	pOut->pInt0 = qkBufferAlignedAlloc(capacity * sizeof(int32_t));
	pOut->pInt1 = qkBufferAlignedAlloc(capacity * sizeof(int32_t));

	pOut->pShort0 = qkBufferAlignedAlloc(capacity * sizeof(int16_t));
	pOut->pShort1 = qkBufferAlignedAlloc(capacity * sizeof(int16_t));
	pOut->pShort2 = qkBufferAlignedAlloc(capacity * sizeof(int16_t));

	if (!pOut->pFloat0 || !pOut->pFloat1 || !pOut->pFloat2 || !pOut->pFloat3 || !pOut->pFloat4 || !pOut->pFloat5 || !pOut->pFloat6 || !pOut->pFloat7 || !pOut->pFloat8 || !pOut->pInt0 || !pOut->pInt1 || !pOut->pShort0 || !pOut->pShort1 || !pOut->pShort2)
	{
		qkBufferDestroy(pOut);
		return 0;
	}

	pOut->capacity = capacity;
	pOut->count	   = 0;
	return 1;
}

void qkBufferDestroy(qkBuffer* pBuffer)
{
	if (!pBuffer)
		return;

	qkBufferAlignedFree(pBuffer->pFloat0);
	qkBufferAlignedFree(pBuffer->pFloat1);
	qkBufferAlignedFree(pBuffer->pFloat2);
	qkBufferAlignedFree(pBuffer->pFloat3);
	qkBufferAlignedFree(pBuffer->pFloat4);
	qkBufferAlignedFree(pBuffer->pFloat5);
	qkBufferAlignedFree(pBuffer->pFloat6);
	qkBufferAlignedFree(pBuffer->pFloat7);
	qkBufferAlignedFree(pBuffer->pFloat8);

	qkBufferAlignedFree(pBuffer->pInt0);
	qkBufferAlignedFree(pBuffer->pInt1);

	qkBufferAlignedFree(pBuffer->pShort0);
	qkBufferAlignedFree(pBuffer->pShort1);
	qkBufferAlignedFree(pBuffer->pShort2);

	memset(pBuffer, 0, sizeof(qkBuffer));
}

void qkBufferClear(qkBuffer* pBuffer)
{
	pBuffer->count = 0;
}

bool qkBufferIsFull(const qkBuffer* pBuffer)
{
	return pBuffer->count >= pBuffer->capacity;
}