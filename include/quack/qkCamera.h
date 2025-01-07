#pragma once

#include "quack/math/qkVec3.h"
#include "quack/qkConstants.h"

typedef struct qkCamera
{
	qkVec3 position;
	float  yaw;	   // Rotation around Y axis
	float  pitch;  // Rotation around X axis
	float  fov;	   // In radians
	float  nearZ;
	float  farZ;
} qkCamera;

void qkCameraInit(qkCamera* camera);
void qkCameraMoveForward(qkCamera* camera, float amount);
void qkCameraMoveRight(qkCamera* camera, float amount);
void qkCameraMoveUp(qkCamera* camera, float amount);
void qkCameraRotate(qkCamera* camera, float yaw, float pitch);
void qkCameraWorldToScreen(const qkCamera* camera, const qkVec3* point, float screenWidth, float screenHeight, qkVec3* out);