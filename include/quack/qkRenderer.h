#pragma once

#include "quack/math/qkVec3.h"
#include "quack/qkTexture.h"

#include <SDL3/SDL.h>

typedef struct qkRenderer
{
	SDL_Window*	  window;
	SDL_Renderer* sdlRenderer;
	SDL_Texture*  frameTexture;
	uint32_t*	  frameBuffer;
	float*		  zBuffer;
	int			  width;
	int			  height;
} qkRenderer;

int	 qkRendererCreate(int width, int height, qkRenderer* out);
void qkRendererDestroy(qkRenderer* renderer);
void qkRendererClear(qkRenderer* renderer);
void qkRendererDrawPixel(qkRenderer* renderer, int x, int y, float z, uint32_t color);
void qkRendererDrawTriangle(
	qkRenderer* renderer, const qkVec3* v1, const qkVec3* v2, const qkVec3* v3, float texU1, float texV1, float texU2, float texV2, float texU3, float texV3, const qkTexture* texture);
void qkRendererPresent(qkRenderer* renderer);