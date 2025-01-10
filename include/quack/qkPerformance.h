#pragma once

#include <stdbool.h>

typedef struct qkPerformance
{
	size_t lastTime;
	size_t frameTime;
	int	   frameCount;
	bool   isWarmUpDone;
	size_t averageFrameTime;
	size_t lastStatsOutput;
} qkPerformance;

void qkPerformanceInit(qkPerformance* perf);
void qkPerformanceUpdate(qkPerformance* perf);