#include "quack/qkRenderer.h"

#include <algorithm>
#include <limits>
#include <stdexcept>

qkRenderer::qkRenderer() : m_window(nullptr), m_renderer(nullptr), m_surface(nullptr), m_texture(nullptr)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		throw std::runtime_error("SDL initialization failed");
	}

	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE);

	m_window = SDL_CreateWindow("Software Renderer", SCREEN_WIDTH, SCREEN_HEIGHT, window_flags);

	if (!m_window)
	{
		throw std::runtime_error("Window creation failed");
	}

	m_renderer = SDL_CreateRenderer(m_window, NULL);

	if (!m_renderer)
	{
		throw std::runtime_error("Renderer creation failed");
	}

	m_surface = SDL_CreateSurface(SCREEN_WIDTH, SCREEN_HEIGHT, SDL_PIXELFORMAT_RGBA8888);
	if (!m_surface)
	{
		throw std::runtime_error("Surface creation failed");
	}

	m_texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
	if (!m_texture)
	{
		throw std::runtime_error("Texture creation failed");
	}

	m_frameBuffer.resize(SCREEN_WIDTH * SCREEN_HEIGHT);
	m_zBuffer.resize(SCREEN_WIDTH * SCREEN_HEIGHT);
}

qkRenderer::~qkRenderer()
{
	if (m_texture)
		SDL_DestroyTexture(m_texture);
	if (m_surface)
		SDL_DestroySurface(m_surface);
	if (m_renderer)
		SDL_DestroyRenderer(m_renderer);
	if (m_window)
		SDL_DestroyWindow(m_window);
	SDL_Quit();
}

void qkRenderer::clearBuffers()
{
	std::fill(m_frameBuffer.begin(), m_frameBuffer.end(), 0);
	std::fill(m_zBuffer.begin(), m_zBuffer.end(), std::numeric_limits<float>::infinity());
}

void qkRenderer::drawPixel(int x, int y, float z, uint32_t color)
{
	if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT)
	{
		return;
	}

	int index = y * SCREEN_WIDTH + x;
	if (z < m_zBuffer[index])
	{
		m_frameBuffer[index] = color;
		m_zBuffer[index]	 = z;
	}
}

void qkRenderer::drawLine(int x1, int y1, int x2, int y2, uint32_t color)
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

void qkRenderer::drawTriangle(const qkVec3& v1, const qkVec3& v2, const qkVec3& v3, float texU1, float texV1, float texU2, float texV2, float texU3, float texV3, const qkTexture* texture)
{
	int minX = std::max(0, static_cast<int>(std::min({v1.x, v2.x, v3.x})));
	int maxX = std::min(SCREEN_WIDTH - 1, static_cast<int>(std::max({v1.x, v2.x, v3.x})));
	int minY = std::max(0, static_cast<int>(std::min({v1.y, v2.y, v3.y})));
	int maxY = std::min(SCREEN_HEIGHT - 1, static_cast<int>(std::max({v1.y, v2.y, v3.y})));

	for (int y = minY; y <= maxY; y++)
	{
		for (int x = minX; x <= maxX; x++)
		{
			float w1 = ((v2.y - v3.y) * (x - v3.x) + (v3.x - v2.x) * (y - v3.y)) / ((v2.y - v3.y) * (v1.x - v3.x) + (v3.x - v2.x) * (v1.y - v3.y));
			float w2 = ((v3.y - v1.y) * (x - v3.x) + (v1.x - v3.x) * (y - v3.y)) / ((v2.y - v3.y) * (v1.x - v3.x) + (v3.x - v2.x) * (v1.y - v3.y));
			float w3 = 1.0f - w1 - w2;

			if (w1 >= 0 && w2 >= 0 && w3 >= 0)
			{
				float z = w1 * v1.z + w2 * v2.z + w3 * v3.z;

				if (texture)
				{
					float	 u	   = w1 * texU1 + w2 * texU2 + w3 * texU3;
					float	 v	   = w1 * texV1 + w2 * texV2 + w3 * texV3;
					uint32_t color = texture->sample(u, v);
					drawPixel(x, y, z, color);
				}
			}
		}
	}
}

void qkRenderer::present()
{
	SDL_UpdateTexture(m_texture, nullptr, m_frameBuffer.data(), SCREEN_WIDTH * sizeof(uint32_t));
	SDL_RenderClear(m_renderer);
	SDL_RenderTexture(m_renderer, m_texture, nullptr, nullptr);
	SDL_RenderPresent(m_renderer);
}