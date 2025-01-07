#pragma once

#include <SDL3/SDL.h>
#include <stdint.h>

typedef struct qkTexture
{
	uint32_t* pixels;
	int		  width;
	int		  height;
} qkTexture;

int		 qkTextureLoad(const char* filename, qkTexture* out);
void	 qkTextureDestroy(qkTexture* texture);
uint32_t qkTextureSample(const qkTexture* texture, float u, float v);