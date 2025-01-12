#include "quack/block/qkTriangleBlock.h"

#include <float.h>
#include <math.h>

int qkTriangleBlockCreate(size_t capacity, qkBlock* pTriangleBlock)
{
	return qkBlockCreate(capacity, pTriangleBlock);
}

void qkTriangleBlockDestroy(qkBlock* pTriangleBlock)
{
	if (!pTriangleBlock)
		return;
	qkBlockDestroy(pTriangleBlock);
}

void qkTriangleBlockClear(qkBlock* pTriangleBlock)
{
	qkBlockClear(pTriangleBlock);
}

int qkTriangleBlockAdd(qkBlock* pTriangleBlock, const qkVec3* pPos1, const qkVec3* pPos2, const qkVec3* pPos3, float u1, float v1, float u2, float v2, float u3, float v3)
{
	if (qkBlockIsFull(pTriangleBlock))
	{
		return 0;
	}

	const size_t idx = pTriangleBlock->count;

	pTriangleBlock->pFloat0[idx]	 = pPos1->x;
	pTriangleBlock->pFloat0[idx + 1] = pPos2->x;
	pTriangleBlock->pFloat0[idx + 2] = pPos3->x;

	pTriangleBlock->pFloat1[idx]	 = pPos1->y;
	pTriangleBlock->pFloat1[idx + 1] = pPos2->y;
	pTriangleBlock->pFloat1[idx + 2] = pPos3->y;

	pTriangleBlock->pFloat2[idx]	 = pPos1->z;
	pTriangleBlock->pFloat2[idx + 1] = pPos2->z;
	pTriangleBlock->pFloat2[idx + 2] = pPos3->z;

	const float invZ1 = 1.0f / pPos1->z;
	const float invZ2 = 1.0f / pPos2->z;
	const float invZ3 = 1.0f / pPos3->z;

	pTriangleBlock->pFloat3[idx]	 = u1 * invZ1;
	pTriangleBlock->pFloat3[idx + 1] = u2 * invZ2;
	pTriangleBlock->pFloat3[idx + 2] = u3 * invZ3;

	pTriangleBlock->pFloat4[idx]	 = v1 * invZ1;
	pTriangleBlock->pFloat4[idx + 1] = v2 * invZ2;
	pTriangleBlock->pFloat4[idx + 2] = v3 * invZ3;

	pTriangleBlock->pFloat5[idx]	 = invZ1;
	pTriangleBlock->pFloat5[idx + 1] = invZ2;
	pTriangleBlock->pFloat5[idx + 2] = invZ3;

	pTriangleBlock->count += 3;
	return 1;
}

void qkTriangleBlockProcess(qkBlock* pTriangleBlock, qkBlock* pSpanBlock, int width, int height, uint32_t* pFrameBuffer, float* pZBuffer, const qkTexture* pTex)
{
	qkSpanBlockClear(pSpanBlock);

	for (size_t i = 0; i < pTriangleBlock->count; i += 3)
	{
		const int	 baseIdx = (int)i * 3;
		const float* pz		 = &pTriangleBlock->pFloat2[baseIdx];

		if (pz[0] < 0.1f || pz[1] < 0.1f || pz[2] < 0.1f)
		{
			continue;
		}

		const float* px = &pTriangleBlock->pFloat0[baseIdx];
		const float* py = &pTriangleBlock->pFloat1[baseIdx];

		const float* puv_u = &pTriangleBlock->pFloat3[baseIdx];
		const float* puv_v = &pTriangleBlock->pFloat4[baseIdx];
		const float* puv_z = &pTriangleBlock->pFloat5[baseIdx];

		qkVec3 edge1 = {px[1] - px[0], py[1] - py[0], pz[1] - pz[0]};
		qkVec3 edge2 = {px[2] - px[0], py[2] - py[0], pz[2] - pz[0]};
		qkVec3 normal;
		qkVec3Cross(&edge2, &edge1, &normal);

		if (normal.z <= 0.f)
			continue;

		float normalLength		= qkVec3Length(&normal);
		float normalizedNormalZ = normal.z / normalLength;

		float maxZ		 = fmaxf(pz[0], fmaxf(pz[1], pz[2]));
		float minZ		 = fminf(pz[0], fminf(pz[1], pz[2]));
		float zRatio	 = minZ / maxZ;
		float screenArea = fabsf((px[1] - px[0]) * (py[2] - py[0]) - (px[2] - px[0]) * (py[1] - py[0])) * 0.5f;

		bool needsPerspective = normalizedNormalZ < 0.95f && zRatio < 0.95f && screenArea > 10000.0f;

		float sortedX[3]	  = {px[0], px[1], px[2]};
		float sortedY[3]	  = {py[0], py[1], py[2]};
		float sortedZ[3]	  = {pz[0], pz[1], pz[2]};
		float sortedUOverZ[3] = {puv_u[0], puv_u[1], puv_u[2]};
		float sortedVOverZ[3] = {puv_v[0], puv_v[1], puv_v[2]};
		float sortedInvZ[3]	  = {puv_z[0], puv_z[1], puv_z[2]};

		for (int j = 0; j < 2; j++)
		{
			for (int k = j + 1; k < 3; k++)
			{
				if (sortedY[j] > sortedY[k])
				{
					float temp;

					temp			= sortedX[j];
					sortedX[j]		= sortedX[k];
					sortedX[k]		= temp;
					temp			= sortedY[j];
					sortedY[j]		= sortedY[k];
					sortedY[k]		= temp;
					temp			= sortedZ[j];
					sortedZ[j]		= sortedZ[k];
					sortedZ[k]		= temp;
					temp			= sortedUOverZ[j];
					sortedUOverZ[j] = sortedUOverZ[k];
					sortedUOverZ[k] = temp;
					temp			= sortedVOverZ[j];
					sortedVOverZ[j] = sortedVOverZ[k];
					sortedVOverZ[k] = temp;
					temp			= sortedInvZ[j];
					sortedInvZ[j]	= sortedInvZ[k];
					sortedInvZ[k]	= temp;
				}
			}
		}

		float yDiff12 = sortedY[1] - sortedY[0];
		float yDiff13 = sortedY[2] - sortedY[0];
		float yDiff23 = sortedY[2] - sortedY[1];

		if (yDiff13 < 0.5f)
			continue;

		float slope1 = (fabsf(yDiff12) < FLT_EPSILON) ? 0.0f : (sortedX[1] - sortedX[0]) / yDiff12;
		float slope2 = (fabsf(yDiff13) < FLT_EPSILON) ? 0.0f : (sortedX[2] - sortedX[0]) / yDiff13;
		float slope3 = (fabsf(yDiff23) < FLT_EPSILON) ? 0.0f : (sortedX[2] - sortedX[1]) / yDiff23;

		float zSlope1 = (fabsf(yDiff12) < FLT_EPSILON) ? 0.0f : (sortedZ[1] - sortedZ[0]) / yDiff12;
		float zSlope2 = (fabsf(yDiff13) < FLT_EPSILON) ? 0.0f : (sortedZ[2] - sortedZ[0]) / yDiff13;
		float zSlope3 = (fabsf(yDiff23) < FLT_EPSILON) ? 0.0f : (sortedZ[2] - sortedZ[1]) / yDiff23;

		float uOverZSlope1 = (fabsf(yDiff12) < FLT_EPSILON) ? 0.0f : (sortedUOverZ[1] - sortedUOverZ[0]) / yDiff12;
		float uOverZSlope2 = (fabsf(yDiff13) < FLT_EPSILON) ? 0.0f : (sortedUOverZ[2] - sortedUOverZ[0]) / yDiff13;
		float uOverZSlope3 = (fabsf(yDiff23) < FLT_EPSILON) ? 0.0f : (sortedUOverZ[2] - sortedUOverZ[1]) / yDiff23;

		float vOverZSlope1 = (fabsf(yDiff12) < FLT_EPSILON) ? 0.0f : (sortedVOverZ[1] - sortedVOverZ[0]) / yDiff12;
		float vOverZSlope2 = (fabsf(yDiff13) < FLT_EPSILON) ? 0.0f : (sortedVOverZ[2] - sortedVOverZ[0]) / yDiff13;
		float vOverZSlope3 = (fabsf(yDiff23) < FLT_EPSILON) ? 0.0f : (sortedVOverZ[2] - sortedVOverZ[1]) / yDiff23;

		float invZSlope1 = (fabsf(yDiff12) < FLT_EPSILON) ? 0.0f : (sortedInvZ[1] - sortedInvZ[0]) / yDiff12;
		float invZSlope2 = (fabsf(yDiff13) < FLT_EPSILON) ? 0.0f : (sortedInvZ[2] - sortedInvZ[0]) / yDiff13;
		float invZSlope3 = (fabsf(yDiff23) < FLT_EPSILON) ? 0.0f : (sortedInvZ[2] - sortedInvZ[1]) / yDiff23;

		int startY = (int)fmaxf(0.0f, ceilf(sortedY[0]));
		int midY   = (int)fminf((float)height - 1, floorf(sortedY[1]));
		int endY   = (int)fminf((float)height - 1, floorf(sortedY[2]));

		// Top half of triangle
		for (int y = startY; y <= midY; y++)
		{
			float dy = (float)y - sortedY[0];

			float leftX	 = sortedX[0] + slope1 * dy;
			float rightX = sortedX[0] + slope2 * dy;
			float leftZ	 = sortedZ[0] + zSlope1 * dy;
			float rightZ = sortedZ[0] + zSlope2 * dy;

			float leftUOverZ  = sortedUOverZ[0] + uOverZSlope1 * dy;
			float rightUOverZ = sortedUOverZ[0] + uOverZSlope2 * dy;
			float leftVOverZ  = sortedVOverZ[0] + vOverZSlope1 * dy;
			float rightVOverZ = sortedVOverZ[0] + vOverZSlope2 * dy;
			float leftInvZ	  = sortedInvZ[0] + invZSlope1 * dy;
			float rightInvZ	  = sortedInvZ[0] + invZSlope2 * dy;

			if (!qkSpanBlockAdd(pSpanBlock, y, leftX, rightX, leftZ, rightZ, leftUOverZ, rightUOverZ, leftVOverZ, rightVOverZ, leftInvZ, rightInvZ, needsPerspective, width))
			{
				qkSpanBlockProcess(pSpanBlock, width, height, pFrameBuffer, pZBuffer, pTex);
				qkSpanBlockClear(pSpanBlock);
				qkSpanBlockAdd(pSpanBlock, y, leftX, rightX, leftZ, rightZ, leftUOverZ, rightUOverZ, leftVOverZ, rightVOverZ, leftInvZ, rightInvZ, needsPerspective, width);
			}
		}

		// Bottom half of triangle
		for (int y = midY + 1; y <= endY; y++)
		{
			float dy1 = (float)y - sortedY[1];
			float dy2 = (float)y - sortedY[0];

			float leftX	 = sortedX[1] + slope3 * dy1;
			float rightX = sortedX[0] + slope2 * dy2;
			float leftZ	 = sortedZ[1] + zSlope3 * dy1;
			float rightZ = sortedZ[0] + zSlope2 * dy2;

			float leftUOverZ  = sortedUOverZ[1] + uOverZSlope3 * dy1;
			float rightUOverZ = sortedUOverZ[0] + uOverZSlope2 * dy2;
			float leftVOverZ  = sortedVOverZ[1] + vOverZSlope3 * dy1;
			float rightVOverZ = sortedVOverZ[0] + vOverZSlope2 * dy2;
			float leftInvZ	  = sortedInvZ[1] + invZSlope3 * dy1;
			float rightInvZ	  = sortedInvZ[0] + invZSlope2 * dy2;

			if (!qkSpanBlockAdd(pSpanBlock, y, leftX, rightX, leftZ, rightZ, leftUOverZ, rightUOverZ, leftVOverZ, rightVOverZ, leftInvZ, rightInvZ, needsPerspective, width))
			{
				qkSpanBlockProcess(pSpanBlock, width, height, pFrameBuffer, pZBuffer, pTex);
				qkSpanBlockClear(pSpanBlock);
				qkSpanBlockAdd(pSpanBlock, y, leftX, rightX, leftZ, rightZ, leftUOverZ, rightUOverZ, leftVOverZ, rightVOverZ, leftInvZ, rightInvZ, needsPerspective, width);
			}
		}
	}

	if (pSpanBlock->count > 0)
		qkSpanBlockProcess(pSpanBlock, width, height, pFrameBuffer, pZBuffer, pTex);

	qkTriangleBlockClear(pTriangleBlock);
}