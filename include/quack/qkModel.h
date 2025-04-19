#pragma once

#include "quack/math/qkVec3.h"

#include <stddef.h>

typedef struct qkVertex
{
	qkVec3 position;
	float  texU;
	float  texV;
} qkVertex;

typedef struct qkTriangle
{
	size_t v1;
	size_t v2;
	size_t v3;
} qkTriangle;

typedef struct qkModel
{
	qkVertex*	pVertices;
	size_t		vertexCount;
	qkTriangle* pTriangles;
	size_t		triangleCount;
	void*		pGltfData;
} qkModel;

int	 qkModelLoad(const char* filename, qkModel* out);
void qkModelDestroy(qkModel* model);