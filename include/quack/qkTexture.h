#pragma once

#include <SDL3/SDL.h>
#include <string>

struct qkTexture
{
	qkTexture(const char* filename);
	~qkTexture();

	uint32_t sample(float u, float v) const;

private:
	SDL_Surface*   m_surface;
	unsigned char* m_data;
};