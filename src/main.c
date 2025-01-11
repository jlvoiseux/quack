#include "quack/qkCamera.h"
#include "quack/qkModel.h"
#include "quack/qkPerformance.h"
#include "quack/qkRenderer.h"
#include "quack/qkTexture.h"

#include <SDL3/SDL.h>
#include <stdio.h>

static void handleInput(qkCamera* camera, int* running)
{
	SDL_Event  event;
	static int lastMouseX = 0;
	static int lastMouseY = 0;
	static int mouseDown  = 0;

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_EVENT_QUIT:
				*running = 0;
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
					qkCameraRotate(camera, deltaX, -deltaY);
					lastMouseX = event.motion.x;
					lastMouseY = event.motion.y;
				}
				break;
		}
	}

	const float moveSpeed = 1.0f;
	const bool* keys	  = SDL_GetKeyboardState(NULL);

	if (keys[SDL_SCANCODE_W])
		qkCameraMoveForward(camera, moveSpeed);
	if (keys[SDL_SCANCODE_S])
		qkCameraMoveForward(camera, -moveSpeed);
	if (keys[SDL_SCANCODE_A])
		qkCameraMoveRight(camera, -moveSpeed);
	if (keys[SDL_SCANCODE_D])
		qkCameraMoveRight(camera, moveSpeed);
	if (keys[SDL_SCANCODE_SPACE])
		qkCameraMoveUp(camera, moveSpeed);
	if (keys[SDL_SCANCODE_LCTRL])
		qkCameraMoveUp(camera, -moveSpeed);
}

static void updateWorldToScreenPositions(const qkCamera* camera, const qkModel* model, qkVec3* screenPositions)
{
	for (size_t i = 0; i < model->vertexCount; i++)
	{
		qkCameraWorldToScreen(camera, &model->pVertices[i].position, QK_SCREEN_WIDTH, QK_SCREEN_HEIGHT, &screenPositions[i]);
	}
}

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
	if (qkModelLoad("assets/autumn_geo.glb", &model) != 0)
	{
		qkRendererDestroy(&renderer);
		return 1;
	}

	qkTexture texture = {0};
	if (qkTextureLoad("assets/autumn_tex.png", &texture) != 0)
	{
		qkModelDestroy(&model);
		qkRendererDestroy(&renderer);
		return 1;
	}

	// Pre-allocate screen position buffer
	qkVec3* screenPositions = malloc(sizeof(qkVec3) * model.vertexCount);
	if (!screenPositions)
	{
		qkTextureDestroy(&texture);
		qkModelDestroy(&model);
		qkRendererDestroy(&renderer);
		return 1;
	}

	qkCamera camera = {0};
	qkCameraInit(&camera);

	qkPerformance perf = {0};
	qkPerformanceInit(&perf);

	int running = 1;
	while (running)
	{
		handleInput(&camera, &running);

		qkRendererClear(&renderer);

		updateWorldToScreenPositions(&camera, &model, screenPositions);

		for (size_t i = 0; i < model.triangleCount; i++)
		{
			const qkTriangle* tri = &model.pTriangles[i];
			const qkVertex*	  v1  = &model.pVertices[tri->v1];
			const qkVertex*	  v2  = &model.pVertices[tri->v2];
			const qkVertex*	  v3  = &model.pVertices[tri->v3];

			qkRendererDrawTriangle(&renderer, &screenPositions[tri->v1], &screenPositions[tri->v2], &screenPositions[tri->v3], v1->texU, v1->texV, v2->texU, v2->texV, v3->texU, v3->texV, &texture);
		}

		qkRendererPresent(&renderer, &texture);
		qkPerformanceUpdate(&perf);
	}

	free(screenPositions);
	qkTextureDestroy(&texture);
	qkModelDestroy(&model);
	qkRendererDestroy(&renderer);

	return 0;
}