#pragma once

#include "quack/math/qkVec3.h"

struct qkCamera
{
	qkCamera();

	void   moveForward(float amount);
	void   moveRight(float amount);
	void   moveUp(float amount);
	void   rotate(float yaw, float pitch);
	qkVec3 worldToScreen(const qkVec3& point, float screenWidth, float screenHeight) const;

	qkVec3 position;
	float  yaw;	   // Rotation around Y axis
	float  pitch;  // Rotation around X axis
	float  fov;	   // In radians
	float  nearZ;
	float  farZ;

private:
	qkVec3 forward() const;
	qkVec3 right() const;
	qkVec3 up() const;
};