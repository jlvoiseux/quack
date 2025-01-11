#include "quack/qkTriangleBatch.h"

#include <float.h>
#include <malloc.h>
#include <string.h>

int qkTriangleBatchCreate(qkTriangleBatch* pBatch)
{
	const size_t vertexCount = QK_BATCH_SIZE * 3;

	pBatch->pPositions = malloc(vertexCount * 3 * sizeof(float));
	pBatch->pTexCoords = malloc(vertexCount * 2 * sizeof(float));
	pBatch->pInvDepths = malloc(vertexCount * sizeof(float));
	pBatch->pIndices   = malloc(QK_BATCH_SIZE * 3 * sizeof(int));

	if (!pBatch->pPositions || !pBatch->pTexCoords || !pBatch->pInvDepths || !pBatch->pIndices)
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
	free(pBatch->pInvDepths);
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
		return 0;

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

	const float invZ1 = 1.0f / pPos1->z;
	const float invZ2 = 1.0f / pPos2->z;
	const float invZ3 = 1.0f / pPos3->z;

	pBatch->pTexCoords[texIdx + 0] = u1 * invZ1;
	pBatch->pTexCoords[texIdx + 1] = v1 * invZ1;
	pBatch->pTexCoords[texIdx + 2] = u2 * invZ2;
	pBatch->pTexCoords[texIdx + 3] = v2 * invZ2;
	pBatch->pTexCoords[texIdx + 4] = u3 * invZ3;
	pBatch->pTexCoords[texIdx + 5] = v3 * invZ3;

	pBatch->pInvDepths[baseIdx + 0] = invZ1;
	pBatch->pInvDepths[baseIdx + 1] = invZ2;
	pBatch->pInvDepths[baseIdx + 2] = invZ3;

	pBatch->pIndices[baseIdx + 0] = baseIdx + 0;
	pBatch->pIndices[baseIdx + 1] = baseIdx + 1;
	pBatch->pIndices[baseIdx + 2] = baseIdx + 2;

	pBatch->count++;
	return 1;
}

void qkTriangleBatchProcess(qkTriangleBatch* pBatch, qkSpanBuffer* pSpanBuffer, int width, int height, uint32_t* pFrameBuffer, float* pZBuffer, const qkTexture* pTex)
{
	for (int i = 0; i < pBatch->count; i++)
	{
		const int baseIdx = i * 3;
		const int posIdx  = baseIdx * 3;
		const int texIdx  = baseIdx * 2;

		if (pBatch->pPositions[posIdx + 2] < 0.1f || pBatch->pPositions[posIdx + 5] < 0.1f || pBatch->pPositions[posIdx + 8] < 0.1f)
		{
			continue;
		}

		qkVec3 p1 = {pBatch->pPositions[posIdx], pBatch->pPositions[posIdx + 1], pBatch->pPositions[posIdx + 2]};
		qkVec3 p2 = {pBatch->pPositions[posIdx + 3], pBatch->pPositions[posIdx + 4], pBatch->pPositions[posIdx + 5]};
		qkVec3 p3 = {pBatch->pPositions[posIdx + 6], pBatch->pPositions[posIdx + 7], pBatch->pPositions[posIdx + 8]};

		const float invZ1 = pBatch->pInvDepths[baseIdx];
		const float invZ2 = pBatch->pInvDepths[baseIdx + 1];
		const float invZ3 = pBatch->pInvDepths[baseIdx + 2];

		float uOverZ1 = pBatch->pTexCoords[texIdx];
		float uOverZ2 = pBatch->pTexCoords[texIdx + 2];
		float uOverZ3 = pBatch->pTexCoords[texIdx + 4];

		float vOverZ1 = pBatch->pTexCoords[texIdx + 1];
		float vOverZ2 = pBatch->pTexCoords[texIdx + 3];
		float vOverZ3 = pBatch->pTexCoords[texIdx + 5];

		qkVec3 edge1, edge2, normal;
		qkVec3Sub(&p2, &p1, &edge1);
		qkVec3Sub(&p3, &p1, &edge2);
		qkVec3Cross(&edge2, &edge1, &normal);

		if (normal.z <= 0.0f)
			continue;

		float normalLength		= qkVec3Length(&normal);
		float normalizedNormalZ = normal.z / normalLength;

		float maxZ		 = fmaxf(p1.z, fmaxf(p2.z, p3.z));
		float minZ		 = fminf(p1.z, fminf(p2.z, p3.z));
		float zRatio	 = minZ / maxZ;
		float screenArea = fabsf((p2.x - p1.x) * (p3.y - p1.y) - (p3.x - p1.x) * (p2.y - p1.y)) * 0.5f;

		bool needsPerspective = normalizedNormalZ < 0.95f || zRatio < 0.95f;

		qkSpanBufferClear(pSpanBuffer);

		const qkVec3* vertices[3] = {&p1, &p2, &p3};
		float		  uOverZ[3]	  = {uOverZ1, uOverZ2, uOverZ3};
		float		  vOverZ[3]	  = {vOverZ1, vOverZ2, vOverZ3};
		float		  invDepth[3] = {invZ1, invZ2, invZ3};

		for (int j = 0; j < 2; j++)
		{
			for (int k = j + 1; k < 3; k++)
			{
				if (vertices[j]->y > vertices[k]->y)
				{
					const qkVec3* tempVec = vertices[j];
					vertices[j]			  = vertices[k];
					vertices[k]			  = tempVec;

					float temp = uOverZ[j];
					uOverZ[j]  = uOverZ[k];
					uOverZ[k]  = temp;

					temp	  = vOverZ[j];
					vOverZ[j] = vOverZ[k];
					vOverZ[k] = temp;

					temp		= invDepth[j];
					invDepth[j] = invDepth[k];
					invDepth[k] = temp;
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

		float invZSlope1 = (fabsf(yDiff12) < FLT_EPSILON) ? 0.0f : (invDepth[1] - invDepth[0]) / yDiff12;
		float invZSlope2 = (fabsf(yDiff13) < FLT_EPSILON) ? 0.0f : (invDepth[2] - invDepth[0]) / yDiff13;
		float invZSlope3 = (fabsf(yDiff23) < FLT_EPSILON) ? 0.0f : (invDepth[2] - invDepth[1]) / yDiff23;

		float uOverZSlope1 = (fabsf(yDiff12) < FLT_EPSILON) ? 0.0f : (uOverZ[1] - uOverZ[0]) / yDiff12;
		float uOverZSlope2 = (fabsf(yDiff13) < FLT_EPSILON) ? 0.0f : (uOverZ[2] - uOverZ[0]) / yDiff13;
		float uOverZSlope3 = (fabsf(yDiff23) < FLT_EPSILON) ? 0.0f : (uOverZ[2] - uOverZ[1]) / yDiff23;

		float vOverZSlope1 = (fabsf(yDiff12) < FLT_EPSILON) ? 0.0f : (vOverZ[1] - vOverZ[0]) / yDiff12;
		float vOverZSlope2 = (fabsf(yDiff13) < FLT_EPSILON) ? 0.0f : (vOverZ[2] - vOverZ[0]) / yDiff13;
		float vOverZSlope3 = (fabsf(yDiff23) < FLT_EPSILON) ? 0.0f : (vOverZ[2] - vOverZ[1]) / yDiff23;

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

			float leftInvZ	  = invDepth[0] + invZSlope1 * dy;
			float rightInvZ	  = invDepth[0] + invZSlope2 * dy;
			float leftUOverZ  = uOverZ[0] + uOverZSlope1 * dy;
			float rightUOverZ = uOverZ[0] + uOverZSlope2 * dy;
			float leftVOverZ  = vOverZ[0] + vOverZSlope1 * dy;
			float rightVOverZ = vOverZ[0] + vOverZSlope2 * dy;

			qkSpanBufferGenerate(pSpanBuffer, y, leftX, rightX, leftZ, rightZ, leftUOverZ, rightUOverZ, leftVOverZ, rightVOverZ, leftInvZ, rightInvZ, needsPerspective, width, height);
		}

		for (int y = midY + 1; y <= endY; y++)
		{
			float dy1 = (float)y - vertices[1]->y;
			float dy2 = (float)y - vertices[0]->y;

			float leftX	 = vertices[1]->x + slope3 * dy1;
			float rightX = vertices[0]->x + slope2 * dy2;
			float leftZ	 = vertices[1]->z + zSlope3 * dy1;
			float rightZ = vertices[0]->z + zSlope2 * dy2;

			float leftInvZ	  = invDepth[1] + invZSlope3 * dy1;
			float rightInvZ	  = invDepth[0] + invZSlope2 * dy2;
			float leftUOverZ  = uOverZ[1] + uOverZSlope3 * dy1;
			float rightUOverZ = uOverZ[0] + uOverZSlope2 * dy2;
			float leftVOverZ  = vOverZ[1] + vOverZSlope3 * dy1;
			float rightVOverZ = vOverZ[0] + vOverZSlope2 * dy2;

			qkSpanBufferGenerate(pSpanBuffer, y, leftX, rightX, leftZ, rightZ, leftUOverZ, rightUOverZ, leftVOverZ, rightVOverZ, leftInvZ, rightInvZ, needsPerspective, width, height);
		}

		qkSpanBufferProcess(pSpanBuffer, width, height, pFrameBuffer, pZBuffer, pTex);
	}

	qkTriangleBatchClear(pBatch);
}