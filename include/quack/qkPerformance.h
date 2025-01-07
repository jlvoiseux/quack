#pragma once

typedef struct qkPerformance
{
	double lastTime;
	double frameTime;
	int	   frameCount;
	double fpsUpdateInterval;
	double lastFpsUpdate;
	float  currentFps;
	float* fpsValues;
	int	   fpsValuesCapacity;
	int	   fpsValuesCount;
} qkPerformance;

void  qkPerformanceInit(qkPerformance* perf);
void  qkPerformanceUpdate(qkPerformance* perf);
float qkPerformanceGetFps(const qkPerformance* perf);
void  qkPerformanceDestroy(qkPerformance* perf);