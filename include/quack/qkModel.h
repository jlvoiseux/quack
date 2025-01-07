#pragma once

#include "quack/math/qkVec3.h"

typedef struct qkVertex
{
	qkVec3 position;
	float  texU;
	float  texV;
} qkVertex;

typedef struct qkTriangle
{
	int v1;
	int v2;
	int v3;
} qkTriangle;

typedef struct qkModel
{
	qkVertex*	vertices;
	size_t		vertexCount;
	qkTriangle* triangles;
	size_t		triangleCount;
	void*		gltfData;
} qkModel;

int	 qkModelLoad(const char* filename, qkModel* out);
void qkModelDestroy(qkModel* model);