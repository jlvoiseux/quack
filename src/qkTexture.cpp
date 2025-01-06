#include "quack/qkTexture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

qkTexture::qkTexture(const char* filename)
{
	int width, height, channels;
	m_data = stbi_load(filename, &width, &height, &channels, 4);

	m_surface = SDL_CreateSurfaceFrom(width, height, SDL_PIXELFORMAT_RGBA8888, m_data, width * 4);
	if (!m_surface)
	{
		stbi_image_free(m_data);
		m_data = nullptr;
	}
}

qkTexture::~qkTexture()
{
	if (m_surface)
	{
		SDL_DestroySurface(m_surface);
	}
	if (m_data)
	{
		stbi_image_free(m_data);
	}
}

uint32_t qkTexture::sample(float u, float v) const
{
	int x = static_cast<int>(u * (m_surface->w - 1) + 0.5f);
	int y = static_cast<int>(v * (m_surface->h - 1) + 0.5f);

	x = (x % m_surface->w + m_surface->w) % m_surface->w;
	y = (y % m_surface->h + m_surface->h) % m_surface->h;

	return static_cast<uint32_t*>(m_surface->pixels)[y * m_surface->w + x];
}