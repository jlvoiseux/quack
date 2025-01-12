#pragma once

#include "qkTexture.h"
#include "quack/block/qkSpanBlock.h"
#include "quack/block/qkTriangleBlock.h"

#include <SDL3/SDL.h>

typedef struct qkRenderer
{
	SDL_Window*	  pWindow;
	SDL_Renderer* pSdlRenderer;
	SDL_Texture*  pFrameTexture;
	uint32_t*	  pFrameBuffer;
	float*		  pZBuffer;
	int			  width;
	int			  height;
	qkBlock		  spanBlock;
	qkBlock		  triangleBlock;
} qkRenderer;

int	 qkRendererCreate(int width, int height, qkRenderer* pOut);
void qkRendererDestroy(qkRenderer* pRenderer);
void qkRendererClear(qkRenderer* pRenderer);
void qkRendererDrawTriangle(qkRenderer* pRenderer, const qkVec3* pPos1, const qkVec3* pPos2, const qkVec3* pPos3, float u1, float v1, float u2, float v2, float u3, float v3, const qkTexture* pTex);
void qkRendererPresent(qkRenderer* pRenderer, const qkTexture* pTex);