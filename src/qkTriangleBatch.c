#include "quack/qkTriangleBatch.h"

#include <float.h>
#include <malloc.h>
#include <string.h>

int qkTriangleBatchCreate(qkTriangleBatch* pBatch)
{
	const size_t vertexCount = QK_BATCH_SIZE * 3;

	pBatch->pPositions = malloc(vertexCount * 3 * sizeof(float));
	pBatch->pTexCoords = malloc(vertexCount * 2 * sizeof(float));
	pBatch->pDepths	   = malloc(vertexCount * sizeof(float));
	pBatch->pIndices   = malloc(QK_BATCH_SIZE * 3 * sizeof(int));

	if (!pBatch->pPositions || !pBatch->pTexCoords || !pBatch->pDepths || !pBatch->pIndices)
	{
		qkTriangleBatchDestroy(pBatch);
		return 0;
	}

	pBatch->count = 0;
	return 1;
}

void qkTriangleBatchDestroy(qkTriangleBatch* pBatch)
{
	if (!pBatch)
		return;

	free(pBatch->pPositions);
	free(pBatch->pTexCoords);
	free(pBatch->pDepths);
	free(pBatch->pIndices);
	memset(pBatch, 0, sizeof(qkTriangleBatch));
}

void qkTriangleBatchClear(qkTriangleBatch* pBatch)
{
	pBatch->count = 0;
}

int qkTriangleBatchAdd(qkTriangleBatch* pBatch, const qkVec3* pPos1, const qkVec3* pPos2, const qkVec3* pPos3, float u1, float v1, float u2, float v2, float u3, float v3)
{
	if (pBatch->count >= QK_BATCH_SIZE)
	{
		return 0;
	}

	const int baseIdx = pBatch->count * 3;
	const int posIdx  = baseIdx * 3;
	const int texIdx  = baseIdx * 2;

	pBatch->pPositions[posIdx + 0] = pPos1->x;
	pBatch->pPositions[posIdx + 1] = pPos1->y;
	pBatch->pPositions[posIdx + 2] = pPos1->z;
	pBatch->pPositions[posIdx + 3] = pPos2->x;
	pBatch->pPositions[posIdx + 4] = pPos2->y;
	pBatch->pPositions[posIdx + 5] = pPos2->z;
	pBatch->pPositions[posIdx + 6] = pPos3->x;
	pBatch->pPositions[posIdx + 7] = pPos3->y;
	pBatch->pPositions[posIdx + 8] = pPos3->z;

	pBatch->pTexCoords[texIdx + 0] = u1;
	pBatch->pTexCoords[texIdx + 1] = v1;
	pBatch->pTexCoords[texIdx + 2] = u2;
	pBatch->pTexCoords[texIdx + 3] = v2;
	pBatch->pTexCoords[texIdx + 4] = u3;
	pBatch->pTexCoords[texIdx + 5] = v3;

	pBatch->pDepths[baseIdx + 0] = pPos1->z;
	pBatch->pDepths[baseIdx + 1] = pPos2->z;
	pBatch->pDepths[baseIdx + 2] = pPos3->z;

	pBatch->pIndices[baseIdx + 0] = baseIdx + 0;
	pBatch->pIndices[baseIdx + 1] = baseIdx + 1;
	pBatch->pIndices[baseIdx + 2] = baseIdx + 2;

	pBatch->count++;
	return 1;
}

void qkTriangleBatchProcess(qkTriangleBatch* pBatch, qkSpanBuffer* pSpanBuffer, int width, int height, uint32_t* frameBuffer, float* pZBuffer, const qkTexture* pTex)
{
	for (int i = 0; i < pBatch->count; i++)
	{
		const int baseIdx = i * 3;
		const int posIdx  = baseIdx * 3;
		const int texIdx  = baseIdx * 2;

		if (pBatch->pDepths[baseIdx] < 0.1f || pBatch->pDepths[baseIdx + 1] < 0.1f || pBatch->pDepths[baseIdx + 2] < 0.1f)
		{
			continue;
		}

		qkVec3 p1 = {pBatch->pPositions[posIdx], pBatch->pPositions[posIdx + 1], pBatch->pPositions[posIdx + 2]};
		qkVec3 p2 = {pBatch->pPositions[posIdx + 3], pBatch->pPositions[posIdx + 4], pBatch->pPositions[posIdx + 5]};
		qkVec3 p3 = {pBatch->pPositions[posIdx + 6], pBatch->pPositions[posIdx + 7], pBatch->pPositions[posIdx + 8]};

		float u1 = pBatch->pTexCoords[texIdx];
		float u2 = pBatch->pTexCoords[texIdx + 2];
		float u3 = pBatch->pTexCoords[texIdx + 4];

		float v1 = pBatch->pTexCoords[texIdx + 1];
		float v2 = pBatch->pTexCoords[texIdx + 3];
		float v3 = pBatch->pTexCoords[texIdx + 5];

		qkVec3 edge1, edge2, normal;
		qkVec3Sub(&p2, &p1, &edge1);
		qkVec3Sub(&p3, &p1, &edge2);
		qkVec3Cross(&edge2, &edge1, &normal);

		if (normal.z <= 0.0f)
		{
			continue;
		}

		qkSpanBufferClear(pSpanBuffer);

		const qkVec3* vertices[3] = {&p1, &p2, &p3};
		float		  texU[3]	  = {u1, u2, u3};
		float		  texV[3]	  = {v1, v2, v3};

		for (int j = 0; j < 2; j++)
		{
			for (int k = j + 1; k < 3; k++)
			{
				if (vertices[j]->y > vertices[k]->y)
				{
					const qkVec3* tempVec = vertices[j];
					vertices[j]			  = vertices[k];
					vertices[k]			  = tempVec;

					float tempU = texU[j];
					texU[j]		= texU[k];
					texU[k]		= tempU;

					float tempV = texV[j];
					texV[j]		= texV[k];
					texV[k]		= tempV;
				}
			}
		}

		float yDiff12 = vertices[1]->y - vertices[0]->y;
		float yDiff13 = vertices[2]->y - vertices[0]->y;
		float yDiff23 = vertices[2]->y - vertices[1]->y;

		if (yDiff13 < 0.5f)
			continue;

		float slope1 = (fabsf(yDiff12) < FLT_EPSILON) ? 0.0f : (vertices[1]->x - vertices[0]->x) / yDiff12;
		float slope2 = (fabsf(yDiff13) < FLT_EPSILON) ? 0.0f : (vertices[2]->x - vertices[0]->x) / yDiff13;
		float slope3 = (fabsf(yDiff23) < FLT_EPSILON) ? 0.0f : (vertices[2]->x - vertices[1]->x) / yDiff23;

		float zSlope1 = (fabsf(yDiff12) < FLT_EPSILON) ? 0.0f : (vertices[1]->z - vertices[0]->z) / yDiff12;
		float zSlope2 = (fabsf(yDiff13) < FLT_EPSILON) ? 0.0f : (vertices[2]->z - vertices[0]->z) / yDiff13;
		float zSlope3 = (fabsf(yDiff23) < FLT_EPSILON) ? 0.0f : (vertices[2]->z - vertices[1]->z) / yDiff23;

		float uSlope1 = (fabsf(yDiff12) < FLT_EPSILON) ? 0.0f : (texU[1] - texU[0]) / yDiff12;
		float uSlope2 = (fabsf(yDiff13) < FLT_EPSILON) ? 0.0f : (texU[2] - texU[0]) / yDiff13;
		float uSlope3 = (fabsf(yDiff23) < FLT_EPSILON) ? 0.0f : (texU[2] - texU[1]) / yDiff23;

		float vSlope1 = (fabsf(yDiff12) < FLT_EPSILON) ? 0.0f : (texV[1] - texV[0]) / yDiff12;
		float vSlope2 = (fabsf(yDiff13) < FLT_EPSILON) ? 0.0f : (texV[2] - texV[0]) / yDiff13;
		float vSlope3 = (fabsf(yDiff23) < FLT_EPSILON) ? 0.0f : (texV[2] - texV[1]) / yDiff23;

		int startY = (int)fmaxf(0.0f, ceilf(vertices[0]->y));
		int midY   = (int)fminf(height - 1, floorf(vertices[1]->y));
		int endY   = (int)fminf(height - 1, floorf(vertices[2]->y));

		for (int y = startY; y <= midY; y++)
		{
			float dy = (float)y - vertices[0]->y;

			float leftX	 = vertices[0]->x + slope1 * dy;
			float rightX = vertices[0]->x + slope2 * dy;
			float leftZ	 = vertices[0]->z + zSlope1 * dy;
			float rightZ = vertices[0]->z + zSlope2 * dy;
			float leftU	 = texU[0] + uSlope1 * dy;
			float rightU = texU[0] + uSlope2 * dy;
			float leftV	 = texV[0] + vSlope1 * dy;
			float rightV = texV[0] + vSlope2 * dy;

			qkSpanBufferGenerate(pSpanBuffer, y, leftX, rightX, leftZ, rightZ, leftU, rightU, leftV, rightV, width, height);
		}

		for (int y = midY + 1; y <= endY; y++)
		{
			float dy1 = (float)y - vertices[1]->y;
			float dy2 = (float)y - vertices[0]->y;

			float leftX	 = vertices[1]->x + slope3 * dy1;
			float rightX = vertices[0]->x + slope2 * dy2;
			float leftZ	 = vertices[1]->z + zSlope3 * dy1;
			float rightZ = vertices[0]->z + zSlope2 * dy2;
			float leftU	 = texU[1] + uSlope3 * dy1;
			float rightU = texU[0] + uSlope2 * dy2;
			float leftV	 = texV[1] + vSlope3 * dy1;
			float rightV = texV[0] + vSlope2 * dy2;

			qkSpanBufferGenerate(pSpanBuffer, y, leftX, rightX, leftZ, rightZ, leftU, rightU, leftV, rightV, width, height);
		}

		qkSpanBufferProcess(pSpanBuffer, width, height, frameBuffer, pZBuffer, pTex);
	}

	qkTriangleBatchClear(pBatch);
}