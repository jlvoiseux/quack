#include "quack/qkCamera.h"
#include "quack/qkModel.h"
#include "quack/qkRenderer.h"
#include "quack/qkTexture.h"

#include <SDL3/SDL.h>

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	qkRenderer renderer = {0};
	if (qkRendererCreate(QK_SCREEN_WIDTH, QK_SCREEN_HEIGHT, &renderer) != 0)
	{
		return 1;
	}

	qkModel model = {0};
	if (qkModelLoad("assets/fox_geo.glb", &model) != 0)
	{
		qkRendererDestroy(&renderer);
		return 1;
	}

	qkTexture texture = {0};
	if (qkTextureLoad("assets/fox_tex.png", &texture) != 0)
	{
		qkModelDestroy(&model);
		qkRendererDestroy(&renderer);
		return 1;
	}

	qkCamera camera = {0};
	qkCameraInit(&camera);

	int		  running = 1;
	SDL_Event event;
	int		  lastMouseX = 0;
	int		  lastMouseY = 0;
	int		  mouseDown	 = 0;

	while (running)
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_EVENT_QUIT:
					running = 0;
					break;

				case SDL_EVENT_MOUSE_BUTTON_DOWN:
					if (event.button.button == SDL_BUTTON_LEFT)
					{
						mouseDown  = 1;
						lastMouseX = event.button.x;
						lastMouseY = event.button.y;
					}
					break;

				case SDL_EVENT_MOUSE_BUTTON_UP:
					if (event.button.button == SDL_BUTTON_LEFT)
					{
						mouseDown = 0;
					}
					break;

				case SDL_EVENT_MOUSE_MOTION:
					if (mouseDown)
					{
						float deltaX = (float)(event.motion.x - lastMouseX) * 0.005f;
						float deltaY = (float)(event.motion.y - lastMouseY) * 0.005f;
						qkCameraRotate(&camera, deltaX, -deltaY);
						lastMouseX = event.motion.x;
						lastMouseY = event.motion.y;
					}
					break;
			}
		}

		const uint8_t* keys		 = SDL_GetKeyboardState(NULL);
		float		   moveSpeed = 1.0f;

		if (keys[SDL_SCANCODE_W])
		{
			qkCameraMoveForward(&camera, moveSpeed);
		}
		if (keys[SDL_SCANCODE_S])
		{
			qkCameraMoveForward(&camera, -moveSpeed);
		}
		if (keys[SDL_SCANCODE_A])
		{
			qkCameraMoveRight(&camera, -moveSpeed);
		}
		if (keys[SDL_SCANCODE_D])
		{
			qkCameraMoveRight(&camera, moveSpeed);
		}
		if (keys[SDL_SCANCODE_Q])
		{
			qkCameraMoveUp(&camera, moveSpeed);
		}
		if (keys[SDL_SCANCODE_E])
		{
			qkCameraMoveUp(&camera, -moveSpeed);
		}

		qkRendererClear(&renderer);

		for (size_t i = 0; i < model.triangleCount; i++)
		{
			qkTriangle tri		  = model.triangles[i];
			qkVec3	   screenPos1 = {0};
			qkVec3	   screenPos2 = {0};
			qkVec3	   screenPos3 = {0};

			qkCameraWorldToScreen(&camera, &model.vertices[tri.v1].position, QK_SCREEN_WIDTH, QK_SCREEN_HEIGHT, &screenPos1);
			qkCameraWorldToScreen(&camera, &model.vertices[tri.v2].position, QK_SCREEN_WIDTH, QK_SCREEN_HEIGHT, &screenPos2);
			qkCameraWorldToScreen(&camera, &model.vertices[tri.v3].position, QK_SCREEN_WIDTH, QK_SCREEN_HEIGHT, &screenPos3);

			qkRendererDrawTriangle(&renderer,
								   &screenPos1,
								   &screenPos2,
								   &screenPos3,
								   model.vertices[tri.v1].texU,
								   model.vertices[tri.v1].texV,
								   model.vertices[tri.v2].texU,
								   model.vertices[tri.v2].texV,
								   model.vertices[tri.v3].texU,
								   model.vertices[tri.v3].texV,
								   &texture);
		}

		qkRendererPresent(&renderer);
	}

	qkTextureDestroy(&texture);
	qkModelDestroy(&model);
	qkRendererDestroy(&renderer);

	return 0;
}