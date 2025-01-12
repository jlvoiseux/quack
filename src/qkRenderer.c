#include "quack/qkRenderer.h"

#include <float.h>
#include <stdio.h>
#include <string.h>

#define QK_SPANS_PER_BLOCK 64
#define QK_VERTEX_PER_BLOCK 64

int qkRendererCreate(int width, int height, qkRenderer* pOut)
{
	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		return -1;
	}

	pOut->pWindow = SDL_CreateWindow("Quack Renderer", width, height, SDL_WINDOW_RESIZABLE);
	if (!pOut->pWindow)
	{
		SDL_Quit();
		return -2;
	}

	pOut->pSdlRenderer = SDL_CreateRenderer(pOut->pWindow, NULL);
	if (!pOut->pSdlRenderer)
	{
		SDL_DestroyWindow(pOut->pWindow);
		SDL_Quit();
		return -3;
	}

	pOut->pFrameTexture = SDL_CreateTexture(pOut->pSdlRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width, height);
	if (!pOut->pFrameTexture)
	{
		SDL_DestroyRenderer(pOut->pSdlRenderer);
		SDL_DestroyWindow(pOut->pWindow);
		SDL_Quit();
		return -4;
	}

#ifdef SIMD_ENABLE
	pOut->pFrameBuffer = _aligned_malloc(width * height * sizeof(uint32_t), 32);
	pOut->pZBuffer	   = _aligned_malloc(width * height * sizeof(float), 32);
#else
	pOut->pFrameBuffer = malloc(width * height * sizeof(uint32_t));
	pOut->pZBuffer	   = malloc(width * height * sizeof(float));
#endif

	if (!pOut->pFrameBuffer || !pOut->pZBuffer)
	{
		if (pOut->pFrameBuffer)
			free(pOut->pFrameBuffer);
		if (pOut->pZBuffer)
			free(pOut->pZBuffer);
		SDL_DestroyTexture(pOut->pFrameTexture);
		SDL_DestroyRenderer(pOut->pSdlRenderer);
		SDL_DestroyWindow(pOut->pWindow);
		SDL_Quit();
		return -5;
	}

	if (!qkSpanBufferCreate(height, QK_SPANS_PER_BLOCK, &pOut->spanBuffer))
	{
		free(pOut->pFrameBuffer);
		free(pOut->pZBuffer);
		SDL_DestroyTexture(pOut->pFrameTexture);
		SDL_DestroyRenderer(pOut->pSdlRenderer);
		SDL_DestroyWindow(pOut->pWindow);
		SDL_Quit();
		return -6;
	}

	if (!qkVertexBufferCreate(QK_VERTEX_PER_BLOCK, &pOut->vertexBuffer0))
	{
		qkSpanBufferDestroy(&pOut->spanBuffer);
		free(pOut->pFrameBuffer);
		free(pOut->pZBuffer);
		SDL_DestroyTexture(pOut->pFrameTexture);
		SDL_DestroyRenderer(pOut->pSdlRenderer);
		SDL_DestroyWindow(pOut->pWindow);
		SDL_Quit();
		return -7;
	}

	if (!qkVertexBufferCreate(QK_VERTEX_PER_BLOCK, &pOut->vertexBuffer1))
	{
		qkSpanBufferDestroy(&pOut->spanBuffer);
		free(pOut->pFrameBuffer);
		free(pOut->pZBuffer);
		SDL_DestroyTexture(pOut->pFrameTexture);
		SDL_DestroyRenderer(pOut->pSdlRenderer);
		SDL_DestroyWindow(pOut->pWindow);
		SDL_Quit();
		return -7;
	}

	if (!qkVertexBufferCreate(QK_VERTEX_PER_BLOCK, &pOut->vertexBuffer2))
	{
		qkSpanBufferDestroy(&pOut->spanBuffer);
		free(pOut->pFrameBuffer);
		free(pOut->pZBuffer);
		SDL_DestroyTexture(pOut->pFrameTexture);
		SDL_DestroyRenderer(pOut->pSdlRenderer);
		SDL_DestroyWindow(pOut->pWindow);
		SDL_Quit();
		return -7;
	}

	pOut->width	 = width;
	pOut->height = height;

	return 0;
}

void qkRendererDestroy(qkRenderer* pRenderer)
{
	if (!pRenderer)
		return;

	qkVertexBufferDestroy(&pRenderer->vertexBuffer0);
	qkVertexBufferDestroy(&pRenderer->vertexBuffer1);
	qkVertexBufferDestroy(&pRenderer->vertexBuffer2);
	qkSpanBufferDestroy(&pRenderer->spanBuffer);

#ifdef SIMD_ENABLE
	_aligned_free(pRenderer->pFrameBuffer);
	_aligned_free(pRenderer->pZBuffer);
#else
	free(pRenderer->pFrameBuffer);
	free(pRenderer->pZBuffer);
#endif

	if (pRenderer->pFrameTexture)
		SDL_DestroyTexture(pRenderer->pFrameTexture);
	if (pRenderer->pSdlRenderer)
		SDL_DestroyRenderer(pRenderer->pSdlRenderer);
	if (pRenderer->pWindow)
		SDL_DestroyWindow(pRenderer->pWindow);

	SDL_Quit();
}

void qkRendererClear(qkRenderer* pRenderer)
{
	memset(pRenderer->pFrameBuffer, 0, pRenderer->width * pRenderer->height * sizeof(uint32_t));
	for (int i = 0; i < pRenderer->width * pRenderer->height; i++)
	{
		pRenderer->pZBuffer[i] = FLT_MAX;
	}
	qkSpanBufferClear(&pRenderer->spanBuffer);
}

void qkRendererDrawTriangle(qkRenderer* pRenderer, const qkVec3* pPos1, const qkVec3* pPos2, const qkVec3* pPos3, float u1, float v1, float u2, float v2, float u3, float v3, const qkTexture* pTex)
{
	if (!qkVertexBufferAdd(&pRenderer->vertexBuffer0, pPos1, u1, v1) || !qkVertexBufferAdd(&pRenderer->vertexBuffer1, pPos2, u2, v2) || !qkVertexBufferAdd(&pRenderer->vertexBuffer2, pPos3, u3, v3))
	{
		qkVertexProcess(&pRenderer->vertexBuffer0, &pRenderer->vertexBuffer1, &pRenderer->vertexBuffer2, &pRenderer->spanBuffer, pRenderer->width, pRenderer->height, pRenderer->pFrameBuffer, pRenderer->pZBuffer, pTex);
		qkVertexBufferAdd(&pRenderer->vertexBuffer0, pPos1, u1, v1);
		qkVertexBufferAdd(&pRenderer->vertexBuffer1, pPos2, u2, v2);
		qkVertexBufferAdd(&pRenderer->vertexBuffer2, pPos3, u3, v3);
	}
}

void qkRendererPresent(qkRenderer* pRenderer, const qkTexture* pTex)
{
	qkVertexProcess(&pRenderer->vertexBuffer0, &pRenderer->vertexBuffer1, &pRenderer->vertexBuffer2, &pRenderer->spanBuffer, pRenderer->width, pRenderer->height, pRenderer->pFrameBuffer, pRenderer->pZBuffer, pTex);

	SDL_UpdateTexture(pRenderer->pFrameTexture, NULL, pRenderer->pFrameBuffer, pRenderer->width * sizeof(uint32_t));
	SDL_RenderClear(pRenderer->pSdlRenderer);
	SDL_RenderTexture(pRenderer->pSdlRenderer, pRenderer->pFrameTexture, NULL, NULL);
	SDL_RenderPresent(pRenderer->pSdlRenderer);
}