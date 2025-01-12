#include "quack/block/qkBlock.h"

#include <stdlib.h>
#include <string.h>

void* qkBlockAlignedAlloc(size_t size)
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

void qkBlockAlignedFree(void* ptr)
{
#ifdef _WIN32
	_aligned_free(ptr);
#else
	free(ptr);
#endif
}

int qkBlockCreate(size_t capacity, qkBlock* pOut)
{
	pOut->pFloat0 = qkBlockAlignedAlloc(capacity * sizeof(float));
	pOut->pFloat1 = qkBlockAlignedAlloc(capacity * sizeof(float));
	pOut->pFloat2 = qkBlockAlignedAlloc(capacity * sizeof(float));
	pOut->pFloat3 = qkBlockAlignedAlloc(capacity * sizeof(float));
	pOut->pFloat4 = qkBlockAlignedAlloc(capacity * sizeof(float));
	pOut->pFloat5 = qkBlockAlignedAlloc(capacity * sizeof(float));
	pOut->pFloat6 = qkBlockAlignedAlloc(capacity * sizeof(float));
	pOut->pFloat7 = qkBlockAlignedAlloc(capacity * sizeof(float));
	pOut->pFloat8 = qkBlockAlignedAlloc(capacity * sizeof(float));

	pOut->pInt0 = qkBlockAlignedAlloc(capacity * sizeof(int32_t));
	pOut->pInt1 = qkBlockAlignedAlloc(capacity * sizeof(int32_t));

	pOut->pShort0 = qkBlockAlignedAlloc(capacity * sizeof(int16_t));
	pOut->pShort1 = qkBlockAlignedAlloc(capacity * sizeof(int16_t));
	pOut->pShort2 = qkBlockAlignedAlloc(capacity * sizeof(int16_t));

	if (!pOut->pFloat0 || !pOut->pFloat1 || !pOut->pFloat2 || !pOut->pFloat3 || !pOut->pFloat4 || !pOut->pFloat5 || !pOut->pFloat6 || !pOut->pFloat7 || !pOut->pFloat8 || !pOut->pInt0 || !pOut->pInt1 || !pOut->pShort0 || !pOut->pShort1 || !pOut->pShort2)
	{
		qkBlockDestroy(pOut);
		return 0;
	}

	pOut->capacity = capacity;
	pOut->count	   = 0;
	return 1;
}

void qkBlockDestroy(qkBlock* pBlock)
{
	if (!pBlock)
		return;

	qkBlockAlignedFree(pBlock->pFloat0);
	qkBlockAlignedFree(pBlock->pFloat1);
	qkBlockAlignedFree(pBlock->pFloat2);
	qkBlockAlignedFree(pBlock->pFloat3);
	qkBlockAlignedFree(pBlock->pFloat4);
	qkBlockAlignedFree(pBlock->pFloat5);
	qkBlockAlignedFree(pBlock->pFloat6);
	qkBlockAlignedFree(pBlock->pFloat7);
	qkBlockAlignedFree(pBlock->pFloat8);

	qkBlockAlignedFree(pBlock->pInt0);
	qkBlockAlignedFree(pBlock->pInt1);

	qkBlockAlignedFree(pBlock->pShort0);
	qkBlockAlignedFree(pBlock->pShort1);
	qkBlockAlignedFree(pBlock->pShort2);

	memset(pBlock, 0, sizeof(qkBlock));
}

void qkBlockClear(qkBlock* pBlock)
{
	pBlock->count = 0;
}

bool qkBlockIsFull(const qkBlock* pBlock)
{
	return pBlock->count >= pBlock->capacity;
}