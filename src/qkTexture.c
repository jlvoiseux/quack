#include "quack/qkTexture.h"

#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int qkTextureLoad(const char* filename, qkTexture* out)
{
	int channels;
	out->pixels = (uint32_t*)stbi_load(filename, &out->width, &out->height, &channels, 4);

	if (!out->pixels)
	{
		return -1;
	}

	return 0;
}

void qkTextureDestroy(qkTexture* texture)
{
	if (!texture)
		return;

	if (texture->pixels)
	{
		stbi_image_free(texture->pixels);
		texture->pixels = NULL;
	}
}

uint32_t qkTextureSample(const qkTexture* texture, float u, float v)
{
	int x = (int)(u * (texture->width - 1) + 0.5f);
	int y = (int)(v * (texture->height - 1) + 0.5f);

	x = (x % texture->width + texture->width) % texture->width;
	y = (y % texture->height + texture->height) % texture->height;

	return texture->pixels[y * texture->width + x];
}