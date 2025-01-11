#include "quack/qkRenderer.h"

#include <float.h>
#include <string.h>

int qkRendererCreate(int width, int height, qkRenderer* pOut)
{
	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		return -1;
	}

	pOut->pWindow = SDL_CreateWindow("Software Renderer", width, height, SDL_WINDOW_RESIZABLE);
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

	pOut->pFrameBuffer = malloc(width * height * sizeof(uint32_t));
	pOut->pZBuffer	   = malloc(width * height * sizeof(float));

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

	if (!qkSpanBufferCreate(height, 64, &pOut->spanBuffer))
	{
		free(pOut->pFrameBuffer);
		free(pOut->pZBuffer);
		SDL_DestroyTexture(pOut->pFrameTexture);
		SDL_DestroyRenderer(pOut->pSdlRenderer);
		SDL_DestroyWindow(pOut->pWindow);
		SDL_Quit();
		return -6;
	}

	if (!qkTriangleBatchCreate(&pOut->triangleBatch))
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

	qkTriangleBatchDestroy(&pRenderer->triangleBatch);
	qkSpanBufferDestroy(&pRenderer->spanBuffer);
	free(pRenderer->pFrameBuffer);
	free(pRenderer->pZBuffer);

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
}

void qkRendererDrawTriangle(qkRenderer* pRenderer, const qkVec3* pPos1, const qkVec3* pPos2, const qkVec3* pPos3, float u1, float v1, float u2, float v2, float u3, float v3, const qkTexture* pTex)
{
	if (!qkTriangleBatchAdd(&pRenderer->triangleBatch, pPos1, pPos2, pPos3, u1, v1, u2, v2, u3, v3))
	{
		qkTriangleBatchProcess(&pRenderer->triangleBatch, &pRenderer->spanBuffer, pRenderer->width, pRenderer->height, pRenderer->pFrameBuffer, pRenderer->pZBuffer, pTex);
		qkTriangleBatchAdd(&pRenderer->triangleBatch, pPos1, pPos2, pPos3, u1, v1, u2, v2, u3, v3);
	}
}

void qkRendererPresent(qkRenderer* pRenderer, const qkTexture* pTex)
{
	qkTriangleBatchProcess(&pRenderer->triangleBatch, &pRenderer->spanBuffer, pRenderer->width, pRenderer->height, pRenderer->pFrameBuffer, pRenderer->pZBuffer, pTex);

	SDL_UpdateTexture(pRenderer->pFrameTexture, NULL, pRenderer->pFrameBuffer, pRenderer->width * sizeof(uint32_t));
	SDL_RenderClear(pRenderer->pSdlRenderer);
	SDL_RenderTexture(pRenderer->pSdlRenderer, pRenderer->pFrameTexture, NULL, NULL);
	SDL_RenderPresent(pRenderer->pSdlRenderer);
}