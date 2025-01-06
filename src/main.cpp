#include "quack/qkCamera.h"
#include "quack/qkDebug.h"
#include "quack/qkModel.h"
#include "quack/qkRenderer.h"

#define _USE_MATH_DEFINES
#include <cmath>
#include <memory>

int main(int argc, char* argv[])
{
	qkRenderer				   renderer;
	std::unique_ptr<qkModel>   model(new qkModel("assets/fox_geo.glb"));
	std::unique_ptr<qkTexture> texture(new qkTexture("assets/fox_tex.png"));
	qkCamera				   camera;

	bool	  running = true;
	SDL_Event event;

	int	 lastMouseX = 0;
	int	 lastMouseY = 0;
	bool mouseDown	= false;

	while (running)
	{
		QK_ZONE_NAMED("Main Loop");

		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_EVENT_QUIT)
			{
				running = false;
			}
			else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
			{
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					mouseDown  = true;
					lastMouseX = event.button.x;
					lastMouseY = event.button.y;
				}
			}
			else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP)
			{
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					mouseDown = false;
				}
			}
			else if (event.type == SDL_EVENT_MOUSE_MOTION && mouseDown)
			{
				float deltaX = (event.motion.x - lastMouseX) * 0.005f;
				float deltaY = (event.motion.y - lastMouseY) * 0.005f;
				camera.rotate(deltaX, -deltaY);
				lastMouseX = event.motion.x;
				lastMouseY = event.motion.y;
			}
		}

		const bool* keys	  = SDL_GetKeyboardState(nullptr);
		float		moveSpeed = 1.f;

		if (keys[SDL_SCANCODE_W])
			camera.moveForward(moveSpeed);
		if (keys[SDL_SCANCODE_S])
			camera.moveForward(-moveSpeed);
		if (keys[SDL_SCANCODE_A])
			camera.moveRight(-moveSpeed);
		if (keys[SDL_SCANCODE_D])
			camera.moveRight(moveSpeed);
		if (keys[SDL_SCANCODE_Q])
			camera.moveUp(moveSpeed);
		if (keys[SDL_SCANCODE_E])
			camera.moveUp(-moveSpeed);

		{
			QK_ZONE_NAMED("Render");

			renderer.clearBuffers();

			std::unique_ptr<qkTexture> texture(new qkTexture("assets/fox_tex.png"));

			for (const auto& triangle : model->triangles)
			{
				qkVec3 v1 = model->vertices[triangle.v1].position;
				qkVec3 v2 = model->vertices[triangle.v2].position;
				qkVec3 v3 = model->vertices[triangle.v3].position;

				v1 = camera.worldToScreen(v1, qkRenderer::SCREEN_WIDTH, qkRenderer::SCREEN_HEIGHT);
				v2 = camera.worldToScreen(v2, qkRenderer::SCREEN_WIDTH, qkRenderer::SCREEN_HEIGHT);
				v3 = camera.worldToScreen(v3, qkRenderer::SCREEN_WIDTH, qkRenderer::SCREEN_HEIGHT);

				renderer.drawTriangle(v1,
									  v2,
									  v3,
									  model->vertices[triangle.v1].texU,
									  model->vertices[triangle.v1].texV,
									  model->vertices[triangle.v2].texU,
									  model->vertices[triangle.v2].texV,
									  model->vertices[triangle.v3].texU,
									  model->vertices[triangle.v3].texV,
									  texture.get());
			}
			renderer.present();
		}

		QK_FRAME_MARK;
	}

	return 0;
}