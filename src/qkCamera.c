#include "quack/qkCamera.h"

#include <math.h>

static void qkCameraForward(const qkCamera* camera, qkVec3* out)
{
	out->x = cosf(camera->yaw) * cosf(camera->pitch);
	out->y = sinf(camera->pitch);
	out->z = sinf(camera->yaw) * cosf(camera->pitch);
	qkVec3Normalize(out, out);
}

static void qkCameraRight(const qkCamera* camera, qkVec3* out)
{
	out->x = -sinf(camera->yaw);
	out->y = 0;
	out->z = cosf(camera->yaw);
	qkVec3Normalize(out, out);
}

static void qkCameraUp(const qkCamera* camera, qkVec3* out)
{
	qkVec3 right, forward;
	qkCameraRight(camera, &right);
	qkCameraForward(camera, &forward);
	qkVec3Cross(&right, &forward, out);
}

void qkCameraInit(qkCamera* camera)
{
	camera->position = qkVec3Create(109.f, 0.8f, 25.5f);
	camera->yaw		 = -2.9f;
	camera->pitch	 = 0.38f;
	camera->fov		 = QK_PI_4;
	camera->nearZ	 = 0.1f;
	camera->farZ	 = 100.0f;
}

void qkCameraMoveForward(qkCamera* camera, float amount)
{
	qkVec3 forward, movement;
	qkCameraForward(camera, &forward);
	qkVec3Scale(&forward, amount, &movement);
	qkVec3Add(&camera->position, &movement, &camera->position);
}

void qkCameraMoveRight(qkCamera* camera, float amount)
{
	qkVec3 right, movement;
	qkCameraRight(camera, &right);
	qkVec3Scale(&right, amount, &movement);
	qkVec3Add(&camera->position, &movement, &camera->position);
}

void qkCameraMoveUp(qkCamera* camera, float amount)
{
	qkVec3 up, movement;
	qkCameraUp(camera, &up);
	qkVec3Scale(&up, amount, &movement);
	qkVec3Add(&camera->position, &movement, &camera->position);
}

void qkCameraRotate(qkCamera* camera, float deltaYaw, float deltaPitch)
{
	camera->yaw += deltaYaw;
	camera->pitch += deltaPitch;

	if (camera->pitch > QK_PI_2 * 0.98f)
	{
		camera->pitch = QK_PI_2 * 0.98f;
	}
	if (camera->pitch < -QK_PI_2 * 0.98f)
	{
		camera->pitch = -QK_PI_2 * 0.98f;
	}
}

void qkCameraWorldToScreen(const qkCamera* camera, const qkVec3* point, float screenWidth, float screenHeight, qkVec3* out)
{
	qkVec3 toPoint, right, up, forward;

	qkVec3Sub(point, &camera->position, &toPoint);
	qkCameraRight(camera, &right);
	qkCameraUp(camera, &up);
	qkCameraForward(camera, &forward);

	float x = qkVec3Dot(&toPoint, &right);
	float y = qkVec3Dot(&toPoint, &up);
	float z = qkVec3Dot(&toPoint, &forward);

	if (z < camera->nearZ)
	{
		*out = qkVec3Create(-1, -1, 0);
		return;
	}

	float aspectRatio = screenWidth / screenHeight;
	float scale		  = tanf(camera->fov * 0.5f);

	x = (x / (z * scale * aspectRatio) * 0.5f + 0.5f) * screenWidth;
	y = (-y / (z * scale) * 0.5f + 0.5f) * screenHeight;

	*out = qkVec3Create(x, y, z);
}