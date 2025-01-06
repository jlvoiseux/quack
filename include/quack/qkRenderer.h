#pragma once

#include "quack/math/qkVec3.h"
#include "quack/qkTexture.h"

#include <SDL3/SDL.h>
#include <vector>

struct qkRenderer
{
	static const int SCREEN_WIDTH  = 1280;
	static const int SCREEN_HEIGHT = 720;

	qkRenderer();
	~qkRenderer();

	void clearBuffers();
	void drawPixel(int x, int y, float z, uint32_t color);
	void drawLine(int x1, int y1, int x2, int y2, uint32_t color);
	void drawTriangle(const qkVec3& v1, const qkVec3& v2, const qkVec3& v3, float texU1, float texV1, float texU2, float texV2, float texU3, float texV3, const qkTexture* texture);
	void present();

private:
	SDL_Window*			  m_window;
	SDL_Renderer*		  m_renderer;
	std::vector<uint32_t> m_frameBuffer;
	std::vector<float>	  m_zBuffer;
	SDL_Surface*		  m_surface;
	SDL_Texture*		  m_texture;
};