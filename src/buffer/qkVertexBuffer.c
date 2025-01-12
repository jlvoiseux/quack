#include "quack/buffer/qkVertexBuffer.h"

#include <float.h>
#include <math.h>

int qkVertexBufferCreate(size_t capacity, qkBuffer* pVertexBuffer)
{
	return qkBufferCreate(capacity, pVertexBuffer);
}

void qkVertexBufferDestroy(qkBuffer* pVertexBuffer)
{
	if (!pVertexBuffer)
		return;
	qkBufferDestroy(pVertexBuffer);
}

void qkVertexBufferClear(qkBuffer* pVertexBuffer)
{
	qkBufferClear(pVertexBuffer);
}

int qkVertexBufferAdd(qkBuffer* pVertexBuffer, const qkVec3* pPos, float u, float v)
{
	if (qkBufferIsFull(pVertexBuffer))
	{
		return 0;
	}

	const size_t idx = pVertexBuffer->count;

	pVertexBuffer->pFloat0[idx] = pPos->x;
	pVertexBuffer->pFloat1[idx] = pPos->y;
	pVertexBuffer->pFloat2[idx] = pPos->z;

	const float invZ = 1.0f / pPos->z;

	pVertexBuffer->pFloat3[idx] = u * invZ;
	pVertexBuffer->pFloat4[idx] = v * invZ;
	pVertexBuffer->pFloat5[idx] = invZ;

	pVertexBuffer->count++;
	return 1;
}

void qkVertexProcess(qkBuffer* pVertexBuffer0, qkBuffer* pVertexBuffer1, qkBuffer* pVertexBuffer2, qkBuffer* pSpanBuffer, int width, int height, uint32_t* pFrameBuffer, float* pZBuffer, const qkTexture* pTex)
{
	qkSpanBufferClear(pSpanBuffer);

	for (size_t i = 0; i < pVertexBuffer0->count; i++)
	{
		float z0 = pVertexBuffer0->pFloat2[i];
		float z1 = pVertexBuffer1->pFloat2[i];
		float z2 = pVertexBuffer2->pFloat2[i];

		if (z0 < 0.1f || z1 < 0.1f || z2 < 0.1f)
		{
			continue;
		}

		float x0 = pVertexBuffer0->pFloat0[i];
		float x1 = pVertexBuffer1->pFloat0[i];
		float x2 = pVertexBuffer2->pFloat0[i];
		float y0 = pVertexBuffer0->pFloat1[i];
		float y1 = pVertexBuffer1->pFloat1[i];
		float y2 = pVertexBuffer2->pFloat1[i];

		float uOverZ0 = pVertexBuffer0->pFloat3[i];
		float uOverZ1 = pVertexBuffer1->pFloat3[i];
		float uOverZ2 = pVertexBuffer2->pFloat3[i];
		float vOverZ0 = pVertexBuffer0->pFloat4[i];
		float vOverZ1 = pVertexBuffer1->pFloat4[i];
		float vOverZ2 = pVertexBuffer2->pFloat4[i];
		float invZ0	  = pVertexBuffer0->pFloat5[i];
		float invZ1	  = pVertexBuffer1->pFloat5[i];
		float invZ2	  = pVertexBuffer2->pFloat5[i];

		qkVec3 edge1 = {x1 - x0, y1 - y0, z1 - z0};
		qkVec3 edge2 = {x2 - x0, y2 - y0, z2 - z0};
		qkVec3 normal;
		qkVec3Cross(&edge2, &edge1, &normal);

		if (normal.z <= 0.0f)
			continue;

		float normalLength		= qkVec3Length(&normal);
		float normalizedNormalZ = normal.z / normalLength;

		float maxZ		 = fmaxf(z0, fmaxf(z1, z2));
		float minZ		 = fminf(z0, fminf(z1, z2));
		float zRatio	 = minZ / maxZ;
		float screenArea = fabsf((x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0)) * 0.5f;

		bool needsPerspective = normalizedNormalZ < 0.95f && zRatio < 0.95f && screenArea > 10000.0f;

		float sortedX[3]	  = {x0, x1, x2};
		float sortedY[3]	  = {y0, y1, y2};
		float sortedZ[3]	  = {z0, z1, z2};
		float sortedUOverZ[3] = {uOverZ0, uOverZ1, uOverZ2};
		float sortedVOverZ[3] = {vOverZ0, vOverZ1, vOverZ2};
		float sortedInvZ[3]	  = {invZ0, invZ1, invZ2};

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

			if (!qkSpanBufferAdd(pSpanBuffer, y, leftX, rightX, leftZ, rightZ, leftUOverZ, rightUOverZ, leftVOverZ, rightVOverZ, leftInvZ, rightInvZ, needsPerspective, width))
			{
#ifdef SIMD_ENABLE
				qkSpanBufferProcess8(pSpanBuffer, width, height, pFrameBuffer, pZBuffer, pTex);
#else
				qkSpanBufferProcess(pSpanBuffer, width, height, pFrameBuffer, pZBuffer, pTex);
#endif
				qkSpanBufferClear(pSpanBuffer);
				qkSpanBufferAdd(pSpanBuffer, y, leftX, rightX, leftZ, rightZ, leftUOverZ, rightUOverZ, leftVOverZ, rightVOverZ, leftInvZ, rightInvZ, needsPerspective, width);
			}
		}

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

			if (!qkSpanBufferAdd(pSpanBuffer, y, leftX, rightX, leftZ, rightZ, leftUOverZ, rightUOverZ, leftVOverZ, rightVOverZ, leftInvZ, rightInvZ, needsPerspective, width))
			{
#ifdef SIMD_ENABLE
				qkSpanBufferProcess8(pSpanBuffer, width, height, pFrameBuffer, pZBuffer, pTex);
#else
				qkSpanBufferProcess(pSpanBuffer, width, height, pFrameBuffer, pZBuffer, pTex);
#endif
				qkSpanBufferClear(pSpanBuffer);
				qkSpanBufferAdd(pSpanBuffer, y, leftX, rightX, leftZ, rightZ, leftUOverZ, rightUOverZ, leftVOverZ, rightVOverZ, leftInvZ, rightInvZ, needsPerspective, width);
			}
		}
	}

	if (pSpanBuffer->count > 0)
	{
#ifdef SIMD_ENABLE
		qkSpanBufferProcess8(pSpanBuffer, width, height, pFrameBuffer, pZBuffer, pTex);
#else
		qkSpanBufferProcess(pSpanBuffer, width, height, pFrameBuffer, pZBuffer, pTex);
#endif
	}

	qkVertexBufferClear(pVertexBuffer0);
	qkVertexBufferClear(pVertexBuffer1);
	qkVertexBufferClear(pVertexBuffer2);
}