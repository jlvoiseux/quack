#include "quack/renderer.h"

#include <algorithm>
#include <limits>
#include <stdexcept>

Renderer::Renderer() : window(nullptr), renderer(nullptr), surface(nullptr), texture(nullptr)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		throw std::runtime_error("SDL initialization failed");
	}

	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE);

	window = SDL_CreateWindow("Software Renderer", SCREEN_WIDTH, SCREEN_HEIGHT, window_flags);

	if (!window)
	{
		throw std::runtime_error("Window creation failed");
	}

	renderer = SDL_CreateRenderer(window, NULL);

	if (!renderer)
	{
		throw std::runtime_error("Renderer creation failed");
	}

	surface = SDL_CreateSurface(SCREEN_WIDTH, SCREEN_HEIGHT, SDL_PIXELFORMAT_RGBA8888);
	if (!surface)
	{
		throw std::runtime_error("Surface creation failed");
	}

	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
	if (!texture)
	{
		throw std::runtime_error("Texture creation failed");
	}

	frameBuffer.resize(SCREEN_WIDTH * SCREEN_HEIGHT);
	zBuffer.resize(SCREEN_WIDTH * SCREEN_HEIGHT);
}

Renderer::~Renderer()
{
	if (texture)
		SDL_DestroyTexture(texture);
	if (surface)
		SDL_DestroySurface(surface);
	if (renderer)
		SDL_DestroyRenderer(renderer);
	if (window)
		SDL_DestroyWindow(window);
	SDL_Quit();
}

void Renderer::clearBuffers()
{
	std::fill(frameBuffer.begin(), frameBuffer.end(), 0);
	std::fill(zBuffer.begin(), zBuffer.end(), std::numeric_limits<float>::infinity());
}

void Renderer::drawPixel(int x, int y, float z, uint32_t color)
{
	if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT)
	{
		return;
	}

	int index = y * SCREEN_WIDTH + x;
	if (z < zBuffer[index])
	{
		frameBuffer[index] = color;
		zBuffer[index]	   = z;
	}
}

void Renderer::drawLine(int x1, int y1, int x2, int y2, uint32_t color)
{
	int dx	= std::abs(x2 - x1);
	int dy	= std::abs(y2 - y1);
	int sx	= x1 < x2 ? 1 : -1;
	int sy	= y1 < y2 ? 1 : -1;
	int err = dx - dy;

	while (true)
	{
		drawPixel(x1, y1, 0, color);

		if (x1 == x2 && y1 == y2)
			break;

		int e2 = 2 * err;
		if (e2 > -dy)
		{
			err -= dy;
			x1 += sx;
		}
		if (e2 < dx)
		{
			err += dx;
			y1 += sy;
		}
	}
}

void Renderer::drawTriangle(const Vec3& v1, const Vec3& v2, const Vec3& v3, uint32_t color)
{
	drawLine(v1.x, v1.y, v2.x, v2.y, color);
	drawLine(v2.x, v2.y, v3.x, v3.y, color);
	drawLine(v3.x, v3.y, v1.x, v1.y, color);
}

void Renderer::present()
{
	SDL_UpdateTexture(texture, nullptr, frameBuffer.data(), SCREEN_WIDTH * sizeof(uint32_t));
	SDL_RenderClear(renderer);
	SDL_RenderTexture(renderer, texture, nullptr, nullptr);
	SDL_RenderPresent(renderer);
}