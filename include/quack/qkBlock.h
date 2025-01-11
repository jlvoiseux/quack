#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct qkBlock
{
	void*  pData;
	size_t elementSize;
	size_t capacity;
	size_t count;
} qkBlock;

int	  qkBlockCreate(size_t elementSize, size_t capacity, qkBlock* pOut);
void  qkBlockDestroy(qkBlock* pBlock);
void  qkBlockClear(qkBlock* pBlock);
bool  qkBlockIsFull(const qkBlock* pBlock);
void* qkBlockGetElement(const qkBlock* pBlock, size_t index);
bool  qkBlockAdd(qkBlock* pBlock, const void* pElement);