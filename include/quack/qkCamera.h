#pragma once

#include "quack/math/qkVec3.h"
#include "quack/qkConstants.h"

typedef struct qkCamera
{
	qkVec3 position;
	float  yaw;
	float  pitch;
	float  fov;
	float  nearZ;
	float  farZ;
} qkCamera;

void qkCameraInit(qkCamera* pCam);
void qkCameraMoveForward(qkCamera* pCam, float amount);
void qkCameraMoveRight(qkCamera* pCam, float amount);
void qkCameraMoveUp(qkCamera* pCam, float amount);
void qkCameraRotate(qkCamera* pCam, float yaw, float pitch);
void qkCameraWorldToScreen(const qkCamera* pCam, const qkVec3* pPoint, float screenWidth, float screenHeight, qkVec3* pOut);