#include "quack/qkPerformance.h"

#include <SDL3/SDL.h>
#include <search.h>
#include <stdio.h>

static int compareFloats(const void* a, const void* b)
{
	float fa = *(const float*)a;
	float fb = *(const float*)b;
	if (fa < fb)
		return -1;
	if (fa > fb)
		return 1;
	return 0;
}

void qkPerformanceInit(qkPerformance* perf)
{
	perf->lastTime			= SDL_GetTicks() / 1000.0;
	perf->frameTime			= 0.0;
	perf->frameCount		= 0;
	perf->fpsUpdateInterval = 1.0;
	perf->lastFpsUpdate		= perf->lastTime;
	perf->currentFps		= 0.0f;
	perf->fpsValuesCapacity = 60;  // Store up to 60 FPS values
	perf->fpsValues			= malloc(perf->fpsValuesCapacity * sizeof(float));
	perf->fpsValuesCount	= 0;
}

void qkPerformanceUpdate(qkPerformance* perf)
{
	double currentTime = SDL_GetTicks() / 1000.0;
	perf->frameTime	   = currentTime - perf->lastTime;
	perf->lastTime	   = currentTime;
	perf->frameCount++;

	if (currentTime - perf->lastFpsUpdate >= perf->fpsUpdateInterval)
	{
		float currentFps = (float)(perf->frameCount / (currentTime - perf->lastFpsUpdate));

		if (perf->fpsValuesCount < perf->fpsValuesCapacity)
		{
			perf->fpsValues[perf->fpsValuesCount++] = currentFps;
		}
		else
		{
			memmove(perf->fpsValues, perf->fpsValues + 1, (perf->fpsValuesCapacity - 1) * sizeof(float));
			perf->fpsValues[perf->fpsValuesCapacity - 1] = currentFps;
		}

		float* sortedFps = malloc(perf->fpsValuesCount * sizeof(float));
		memcpy(sortedFps, perf->fpsValues, perf->fpsValuesCount * sizeof(float));
		qsort(sortedFps, perf->fpsValuesCount, sizeof(float), compareFloats);

		float medianFps = sortedFps[perf->fpsValuesCount / 2];
		free(sortedFps);

		printf("\rMedian FPS: %.5f", medianFps);
		fflush(stdout);

		perf->frameCount	= 0;
		perf->lastFpsUpdate = currentTime;
	}
}

float qkPerformanceGetFps(const qkPerformance* perf)
{
	if (perf->fpsValuesCount == 0)
		return 0.0f;

	float* sortedFps = malloc(perf->fpsValuesCount * sizeof(float));
	memcpy(sortedFps, perf->fpsValues, perf->fpsValuesCount * sizeof(float));
	qsort(sortedFps, perf->fpsValuesCount, sizeof(float), compareFloats);

	float medianFps = sortedFps[perf->fpsValuesCount / 2];
	free(sortedFps);

	return medianFps;
}

void qkPerformanceDestroy(qkPerformance* perf)
{
	if (perf && perf->fpsValues)
	{
		free(perf->fpsValues);
		perf->fpsValues = NULL;
	}
}