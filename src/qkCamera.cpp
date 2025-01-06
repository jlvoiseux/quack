#include "quack/qkCamera.h"

#define _USE_MATH_DEFINES
#include <cmath>

qkCamera::qkCamera() : position(0, 0, -5), yaw(0), pitch(0), fov(M_PI / 4.0f), nearZ(0.1f), farZ(100.0f) {}

void qkCamera::moveForward(float amount)
{
	position = position + forward() * amount;
}

void qkCamera::moveRight(float amount)
{
	position = position + right() * amount;
}

void qkCamera::moveUp(float amount)
{
	position = position + up() * amount;
}

void qkCamera::rotate(float deltaYaw, float deltaPitch)
{
	yaw += deltaYaw;
	pitch += deltaPitch;

	// Clamp pitch to avoid gimbal lock
	if (pitch > M_PI * 0.49f)
		pitch = M_PI * 0.49f;
	if (pitch < -M_PI * 0.49f)
		pitch = -M_PI * 0.49f;
}

qkVec3 qkCamera::forward() const
{
	return qkVec3(cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch)).normalized();
}

qkVec3 qkCamera::right() const
{
	return qkVec3(-sin(yaw), 0, cos(yaw)).normalized();
}

qkVec3 qkCamera::up() const
{
	return right().cross(forward());
}

qkVec3 qkCamera::worldToScreen(const qkVec3& point, float screenWidth, float screenHeight) const
{
	// Transform point to camera space
	qkVec3 toPoint = point - position;

	// Project to camera's view plane
	float x = toPoint.dot(right());
	float y = toPoint.dot(up());
	float z = toPoint.dot(forward());

	// Early exit if behind camera
	if (z < nearZ)
		return qkVec3(-1, -1, 0);

	// Perspective projection
	float aspectRatio = screenWidth / screenHeight;
	float scale		  = tan(fov * 0.5f);

	x = (x / (z * scale * aspectRatio) * 0.5f + 0.5f) * screenWidth;
	y = (-y / (z * scale) * 0.5f + 0.5f) * screenHeight;

	return qkVec3(x, y, z);
}