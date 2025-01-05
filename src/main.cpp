#include "quack/renderer.h"

#define _USE_MATH_DEFINES
#include <cmath>

int main(int argc, char* argv[])
{
	try
	{
		Renderer  renderer;
		bool	  running = true;
		SDL_Event event;

		while (running)
		{
			while (SDL_PollEvent(&event))
			{
				if (event.type == SDL_EVENT_QUIT)
				{
					running = false;
				}
			}

			renderer.clearBuffers();

			static float angle	 = 0.0f;
			float		 size	 = 100.0f;
			float		 centerX = Renderer::SCREEN_WIDTH / 2.0f;
			float		 centerY = Renderer::SCREEN_HEIGHT / 2.0f;

			Vec3 v1(centerX + size * std::cos(angle), centerY + size * std::sin(angle), 0);
			Vec3 v2(centerX + size * std::cos(angle + 2.0f * M_PI / 3.0f), centerY + size * std::sin(angle + 2.0f * M_PI / 3.0f), 0);
			Vec3 v3(centerX + size * std::cos(angle + 4.0f * M_PI / 3.0f), centerY + size * std::sin(angle + 4.0f * M_PI / 3.0f), 0);

			renderer.drawTriangle(v1, v2, v3, 0xFFFFFFFF);

			renderer.present();
			angle += 0.01f;

			SDL_Delay(16);
		}

		return 0;
	}
	catch (const std::exception& e)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error: %s\n", e.what());
		return 1;
	}
}