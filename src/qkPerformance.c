#include "quack/qkPerformance.h"

#include "quack/qkConstants.h"

#include <SDL3/SDL.h>
#include <stdio.h>

void qkPerformanceInit(qkPerformance* perf)
{
	perf->frameTime		   = 0;
	perf->frameCount	   = 0;
	perf->isWarmUpDone	   = false;
	perf->averageFrameTime = 0;
	perf->lastStatsOutput  = perf->lastTime;
}

void qkPerformanceUpdate(qkPerformance* perf)
{
	const size_t currentTime = SDL_GetTicks();
	perf->frameTime			 = currentTime - perf->lastTime;
	perf->lastTime			 = currentTime;

	if (!perf->isWarmUpDone)
	{
		if (currentTime >= QK_WARM_UP_TIME)
		{
			perf->isWarmUpDone	   = true;
			perf->frameCount	   = 0;
			perf->averageFrameTime = 0;
			printf("Warm-up complete. Starting performance measurements.\n");
			fflush(stdout);
			perf->lastStatsOutput = currentTime;
		}
		return;
	}

	perf->frameCount++;
	perf->averageFrameTime = (float)(perf->averageFrameTime * ((float)perf->frameCount - 1) + (float)perf->frameTime) / (float)perf->frameCount;

	if (currentTime - perf->lastStatsOutput >= QK_STATS_INTERVAL)
	{
		const float avgFps = perf->averageFrameTime > 0 ? 1000.0f / perf->averageFrameTime : 0.0f;
		printf("Stats | Avg: %.4f ms (%.4f FPS) over %d frames, Curr: %zu ms\n", perf->averageFrameTime, avgFps, perf->frameCount, perf->frameTime);
		fflush(stdout);
		perf->lastStatsOutput = currentTime;
	}
}