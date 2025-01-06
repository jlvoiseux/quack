#pragma once

#include "quack/math/qkVec3.h"

#include <string>
#include <vector>

struct qkModel
{
	struct Vertex
	{
		qkVec3 position;
		float  texU;
		float  texV;
	};

	struct Triangle
	{
		int v1;
		int v2;
		int v3;
	};

	qkModel(const char* filename);
	~qkModel();

	std::vector<Vertex>	  vertices;
	std::vector<Triangle> triangles;

private:
	void* m_gltfData;
};