#include "quack/qkBlock.h"

#include <stdlib.h>
#include <string.h>

int qkBlockCreate(size_t elementSize, size_t capacity, qkBlock* pOut)
{
	pOut->pData = malloc(elementSize * capacity);
	if (!pOut->pData)
	{
		return 0;
	}

	pOut->elementSize = elementSize;
	pOut->capacity	  = capacity;
	pOut->count		  = 0;
	return 1;
}

void qkBlockDestroy(qkBlock* pBlock)
{
	free(pBlock->pData);
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

void* qkBlockGetElement(const qkBlock* pBlock, size_t index)
{
	if (index >= pBlock->count)
	{
		return NULL;
	}
	return (char*)pBlock->pData + (index * pBlock->elementSize);
}

bool qkBlockAdd(qkBlock* pBlock, const void* pElement)
{
	if (qkBlockIsFull(pBlock))
	{
		return false;
	}

	memcpy((char*)pBlock->pData + (pBlock->count * pBlock->elementSize), pElement, pBlock->elementSize);
	pBlock->count++;
	return true;
}