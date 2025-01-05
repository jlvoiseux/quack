#pragma once

#include "quack/math/vec3.h"

#include <SDL3/SDL.h>
#include <vector>

struct Renderer
{
	static const int SCREEN_WIDTH  = 800;
	static const int SCREEN_HEIGHT = 600;

	Renderer();
	~Renderer();

	void clearBuffers();
	void drawPixel(int x, int y, float z, uint32_t color);
	void drawLine(int x1, int y1, int x2, int y2, uint32_t color);
	void drawTriangle(const Vec3& v1, const Vec3& v2, const Vec3& v3, uint32_t color);
	void present();

  private:
	SDL_Window*			  window;
	SDL_Renderer*		  renderer;
	std::vector<uint32_t> frameBuffer;
	std::vector<float>	  zBuffer;
	SDL_Surface*		  surface;
	SDL_Texture*		  texture;
};