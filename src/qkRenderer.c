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

void qkRendererDrawPixel(qkRenderer* renderer, int x, int y, float z, uint32_t color)
{
	if (x < 0 || x >= renderer->width || y < 0 || y >= renderer->height)
		return;

	int index = y * renderer->width + x;
	if (z < renderer->zBuffer[index])
	{
		renderer->frameBuffer[index] = color;
		renderer->zBuffer[index]	 = z;
	}
}

void qkRendererDrawTriangle(
	qkRenderer* renderer, const qkVec3* v1, const qkVec3* v2, const qkVec3* v3, float texU1, float texV1, float texU2, float texV2, float texU3, float texV3, const qkTexture* texture)
{
	int minX = (int)fmaxf(0.0f, fminf(fminf(v1->x, v2->x), v3->x));
	int maxX = (int)fminf(renderer->width - 1, fmaxf(fmaxf(v1->x, v2->x), v3->x));
	int minY = (int)fmaxf(0.0f, fminf(fminf(v1->y, v2->y), v3->y));
	int maxY = (int)fminf(renderer->height - 1, fmaxf(fmaxf(v1->y, v2->y), v3->y));

	for (int y = minY; y <= maxY; y++)
	{
		for (int x = minX; x <= maxX; x++)
		{
			float w1 = ((v2->y - v3->y) * (x - v3->x) + (v3->x - v2->x) * (y - v3->y)) / ((v2->y - v3->y) * (v1->x - v3->x) + (v3->x - v2->x) * (v1->y - v3->y));
			float w2 = ((v3->y - v1->y) * (x - v3->x) + (v1->x - v3->x) * (y - v3->y)) / ((v2->y - v3->y) * (v1->x - v3->x) + (v3->x - v2->x) * (v1->y - v3->y));
			float w3 = 1.0f - w1 - w2;

			if (w1 >= 0 && w2 >= 0 && w3 >= 0)
			{
				float z = w1 * v1->z + w2 * v2->z + w3 * v3->z;

				if (texture)
				{
					float	 u	   = w1 * texU1 + w2 * texU2 + w3 * texU3;
					float	 v	   = w1 * texV1 + w2 * texV2 + w3 * texV3;
					uint32_t color = qkTextureSample(texture, u, v);
					qkRendererDrawPixel(renderer, x, y, z, color);
				}
			}
		}
	}
}

void qkRendererPresent(qkRenderer* renderer)
{
	SDL_UpdateTexture(renderer->frameTexture, NULL, renderer->frameBuffer, renderer->width * sizeof(uint32_t));
	SDL_RenderClear(renderer->sdlRenderer);
	SDL_RenderTexture(renderer->sdlRenderer, renderer->frameTexture, NULL, NULL);
	SDL_RenderPresent(renderer->sdlRenderer);
}