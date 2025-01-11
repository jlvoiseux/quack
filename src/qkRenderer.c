#include "quack/qkRenderer.h"

#include <float.h>
#include <stdlib.h>
#include <string.h>

int qkRendererCreate(int width, int height, qkRenderer* out)
{
	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		return -1;
	}

	out->window = SDL_CreateWindow("Software Renderer", width, height, SDL_WINDOW_RESIZABLE);
	if (!out->window)
	{
		SDL_Quit();
		return -2;
	}

	out->sdlRenderer = SDL_CreateRenderer(out->window, NULL);
	if (!out->sdlRenderer)
	{
		SDL_DestroyWindow(out->window);
		SDL_Quit();
		return -3;
	}

	out->frameTexture = SDL_CreateTexture(out->sdlRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width, height);
	if (!out->frameTexture)
	{
		SDL_DestroyRenderer(out->sdlRenderer);
		SDL_DestroyWindow(out->window);
		SDL_Quit();
		return -4;
	}

	out->frameBuffer = malloc(width * height * sizeof(uint32_t));
	out->zBuffer	 = malloc(width * height * sizeof(float));

	if (!out->frameBuffer || !out->zBuffer)
	{
		if (out->frameBuffer)
			free(out->frameBuffer);
		if (out->zBuffer)
			free(out->zBuffer);
		SDL_DestroyTexture(out->frameTexture);
		SDL_DestroyRenderer(out->sdlRenderer);
		SDL_DestroyWindow(out->window);
		SDL_Quit();
		return -5;
	}

	if (qkSpanBufferCreate(height, 64, &out->spanBuffer) != 0)
	{
		free(out->frameBuffer);
		free(out->zBuffer);
		SDL_DestroyTexture(out->frameTexture);
		SDL_DestroyRenderer(out->sdlRenderer);
		SDL_DestroyWindow(out->window);
		SDL_Quit();
		return -6;
	}

	out->width	= width;
	out->height = height;

	return 0;
}

void qkRendererDestroy(qkRenderer* renderer)
{
	if (!renderer)
		return;

	free(renderer->frameBuffer);
	free(renderer->zBuffer);
	qkSpanBufferDestroy(&renderer->spanBuffer);

	if (renderer->frameTexture)
		SDL_DestroyTexture(renderer->frameTexture);
	if (renderer->sdlRenderer)
		SDL_DestroyRenderer(renderer->sdlRenderer);
	if (renderer->window)
		SDL_DestroyWindow(renderer->window);

	SDL_Quit();
}

void qkRendererClear(qkRenderer* renderer)
{
	memset(renderer->frameBuffer, 0, renderer->width * renderer->height * sizeof(uint32_t));
	for (int i = 0; i < renderer->width * renderer->height; i++)
	{
		renderer->zBuffer[i] = FLT_MAX;
	}
}

void qkRendererDrawTriangle(
	qkRenderer* renderer, const qkVec3* v1, const qkVec3* v2, const qkVec3* v3, float texU1, float texV1, float texU2, float texV2, float texU3, float texV3, const qkTexture* texture)
{
	// Back-face culling
	qkVec3 edge1, edge2, normal;
	qkVec3Sub(v2, v1, &edge1);
	qkVec3Sub(v3, v1, &edge2);
	qkVec3Cross(&edge2, &edge1, &normal);

	if (normal.z <= 0.0f)
	{
		return;
	}

	// Near plane clipping
	if (v1->z < 0.1f || v2->z < 0.1f || v3->z < 0.1f)
	{
		return;
	}

	qkSpanBufferClear(&renderer->spanBuffer);

	// Sort vertices by Y coordinate
	const qkVec3* vertices[3] = {v1, v2, v3};
	float		  texU[3]	  = {texU1, texU2, texU3};
	float		  texV[3]	  = {texV1, texV2, texV3};

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2 - i; j++)
		{
			if (vertices[j]->y > vertices[j + 1]->y)
			{
				const qkVec3* tempVec = vertices[j];
				vertices[j]			  = vertices[j + 1];
				vertices[j + 1]		  = tempVec;

				float tempU = texU[j];
				texU[j]		= texU[j + 1];
				texU[j + 1] = tempU;

				float tempV = texV[j];
				texV[j]		= texV[j + 1];
				texV[j + 1] = tempV;
			}
		}
	}

	v1	  = vertices[0];
	v2	  = vertices[1];
	v3	  = vertices[2];
	texU1 = texU[0];
	texU2 = texU[1];
	texU3 = texU[2];
	texV1 = texV[0];
	texV2 = texV[1];
	texV3 = texV[2];

	// Generate spans for the triangle
	float yDiff12 = v2->y - v1->y;
	float yDiff13 = v3->y - v1->y;
	float yDiff23 = v3->y - v2->y;

	float slope1 = (fabsf(yDiff12) < FLT_EPSILON) ? 0.0f : (v2->x - v1->x) / yDiff12;
	float slope2 = (fabsf(yDiff13) < FLT_EPSILON) ? 0.0f : (v3->x - v1->x) / yDiff13;
	float slope3 = (fabsf(yDiff23) < FLT_EPSILON) ? 0.0f : (v3->x - v2->x) / yDiff23;

	float zSlope1 = (fabsf(yDiff12) < FLT_EPSILON) ? 0.0f : (v2->z - v1->z) / yDiff12;
	float zSlope2 = (fabsf(yDiff13) < FLT_EPSILON) ? 0.0f : (v3->z - v1->z) / yDiff13;
	float zSlope3 = (fabsf(yDiff23) < FLT_EPSILON) ? 0.0f : (v3->z - v2->z) / yDiff23;

	float uSlope1 = (fabsf(yDiff12) < FLT_EPSILON) ? 0.0f : (texU2 - texU1) / yDiff12;
	float uSlope2 = (fabsf(yDiff13) < FLT_EPSILON) ? 0.0f : (texU3 - texU1) / yDiff13;
	float uSlope3 = (fabsf(yDiff23) < FLT_EPSILON) ? 0.0f : (texU3 - texU2) / yDiff23;

	float vSlope1 = (fabsf(yDiff12) < FLT_EPSILON) ? 0.0f : (texV2 - texV1) / yDiff12;
	float vSlope2 = (fabsf(yDiff13) < FLT_EPSILON) ? 0.0f : (texV3 - texV1) / yDiff13;
	float vSlope3 = (fabsf(yDiff23) < FLT_EPSILON) ? 0.0f : (texV3 - texV2) / yDiff23;

	int startY = (int)fmaxf(0.0f, ceilf(v1->y));
	int endY   = (int)fminf(renderer->height - 1, floorf(v3->y));

	for (int y = startY; y <= endY; y++)
	{
		float dy		  = (float)y - v1->y;
		int	  isUpperHalf = y < v2->y;

		float leftX, rightX, leftZ, rightZ, leftU, rightU, leftV, rightV;

		if (isUpperHalf)
		{
			leftX = v1->x + slope1 * dy;
			leftZ = v1->z + zSlope1 * dy;
			leftU = texU1 + uSlope1 * dy;
			leftV = texV1 + vSlope1 * dy;
		}
		else
		{
			float dy2 = (float)y - v2->y;
			leftX	  = v2->x + slope3 * dy2;
			leftZ	  = v2->z + zSlope3 * dy2;
			leftU	  = texU2 + uSlope3 * dy2;
			leftV	  = texV2 + vSlope3 * dy2;
		}

		rightX = v1->x + slope2 * dy;
		rightZ = v1->z + zSlope2 * dy;
		rightU = texU1 + uSlope2 * dy;
		rightV = texV1 + vSlope2 * dy;

		if (leftX > rightX)
		{
			float temp;
			temp   = leftX;
			leftX  = rightX;
			rightX = temp;
			temp   = leftZ;
			leftZ  = rightZ;
			rightZ = temp;
			temp   = leftU;
			leftU  = rightU;
			rightU = temp;
			temp   = leftV;
			leftV  = rightV;
			rightV = temp;
		}

		int startX = (int)fmaxf(0.0f, ceilf(leftX));
		int endX   = (int)fminf(renderer->width - 1, floorf(rightX));

		if (startX <= endX)
		{
			qkSpanBufferAddSpan(&renderer->spanBuffer, y, startX, endX, leftZ, rightZ, leftU, rightU, leftV, rightV);
		}
	}

	// Process all spans
	for (int y = 0; y < renderer->height; y++)
	{
		int		spanCount = renderer->spanBuffer.spanCounts[y];
		qkSpan* spans	  = &renderer->spanBuffer.spans[y * renderer->spanBuffer.maxSpansPerLine];

		for (int i = 0; i < spanCount; i++)
		{
			qkSpan* span  = &spans[i];
			int		width = span->endX - span->startX + 1;
			if (width <= 0)
				continue;

			float zStep = (span->endZ - span->startZ) / width;
			float uStep = (span->endU - span->startU) / width;
			float vStep = (span->endV - span->startV) / width;

			float z = span->startZ;
			float u = span->startU;
			float v = span->startV;

			// Process span pixels linearly
			int offset = y * renderer->width + span->startX;
			for (int x = 0; x < width; x++)
			{
				if (z < renderer->zBuffer[offset])
				{
					renderer->frameBuffer[offset] = qkTextureSample(texture, u, v);
					renderer->zBuffer[offset]	  = z;
				}
				z += zStep;
				u += uStep;
				v += vStep;
				offset++;
			}
		}
	}
}

void qkRendererPresent(qkRenderer* renderer, const qkTexture* texture)
{
	SDL_UpdateTexture(renderer->frameTexture, NULL, renderer->frameBuffer, renderer->width * sizeof(uint32_t));
	SDL_RenderClear(renderer->sdlRenderer);
	SDL_RenderTexture(renderer->sdlRenderer, renderer->frameTexture, NULL, NULL);
	SDL_RenderPresent(renderer->sdlRenderer);
}