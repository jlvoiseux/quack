#include "quack/qkPerformance.h"

#include <SDL3/SDL.h>
#include <stdio.h>

#define QK_WARM_UP_TIME 2.0
#define QK_STATS_INTERVAL 5.0

void qkPerformanceInit(qkPerformance* perf)
{
	perf->frameTime		   = 0.0;
	perf->frameCount	   = 0;
	perf->isWarmUpDone	   = false;
	perf->averageFrameTime = 0.0;
	perf->lastStatsOutput  = perf->lastTime;
}

void qkPerformanceUpdate(qkPerformance* perf)
{
	const double currentTime = SDL_GetTicks() / 1000.0;
	perf->frameTime			 = currentTime - perf->lastTime;
	perf->lastTime			 = currentTime;

	if (!perf->isWarmUpDone)
	{
		if (currentTime >= QK_WARM_UP_TIME)
		{
			perf->isWarmUpDone	   = true;
			perf->frameCount	   = 0;
			perf->averageFrameTime = 0.0;
			printf("Warm-up complete. Starting performance measurements.\n");
		}
		return;
	}

	perf->frameCount++;
	perf->averageFrameTime = (perf->averageFrameTime * (perf->frameCount - 1) + perf->frameTime) / perf->frameCount;

	if (currentTime - perf->lastStatsOutput >= QK_STATS_INTERVAL)
	{
		const float avgFps = perf->averageFrameTime > 0.0 ? 1.0f / (float)perf->averageFrameTime : 0.0f;
		printf("Average frame time: %.4f ms (%.4f FPS) over %d frames\n", perf->averageFrameTime * 1000.0, avgFps, perf->frameCount);
		perf->lastStatsOutput = currentTime;
	}
}