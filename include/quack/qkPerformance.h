#pragma once

#include <stdbool.h>

typedef struct qkPerformance
{
	size_t lastTime;
	size_t frameTime;
	int	   frameCount;
	bool   isWarmUpDone;
	float  averageFrameTime;
	size_t lastStatsOutput;
} qkPerformance;

void qkPerformanceInit(qkPerformance* perf);
void qkPerformanceUpdate(qkPerformance* perf);