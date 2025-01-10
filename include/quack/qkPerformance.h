#pragma once

#include <stdbool.h>

typedef struct qkPerformance
{
	double lastTime;
	double frameTime;
	int	   frameCount;
	bool   isWarmUpDone;
	double averageFrameTime;
	double lastStatsOutput;
} qkPerformance;

void qkPerformanceInit(qkPerformance* perf);
void qkPerformanceUpdate(qkPerformance* perf);