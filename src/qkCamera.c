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

void qkCameraInit(qkCamera* pCam)
{
	pCam->position = qkVec3Create(116.22f, 7.65f, -7.5f);
	pCam->yaw	   = -2.875f;
	pCam->pitch	   = -0.08f;
	pCam->fov	   = QK_PI_4;
	pCam->nearZ	   = 0.1f;
	pCam->farZ	   = 100.0f;
}

void qkCameraMoveForward(qkCamera* pCam, float amount)
{
	qkVec3 forward, movement;
	qkCameraForward(pCam, &forward);
	qkVec3Scale(&forward, amount, &movement);
	qkVec3Add(&pCam->position, &movement, &pCam->position);
}

void qkCameraMoveRight(qkCamera* pCam, float amount)
{
	qkVec3 right, movement;
	qkCameraRight(pCam, &right);
	qkVec3Scale(&right, amount, &movement);
	qkVec3Add(&pCam->position, &movement, &pCam->position);
}

void qkCameraMoveUp(qkCamera* pCam, float amount)
{
	qkVec3 up, movement;
	qkCameraUp(pCam, &up);
	qkVec3Scale(&up, amount, &movement);
	qkVec3Add(&pCam->position, &movement, &pCam->position);
}

void qkCameraRotate(qkCamera* pCam, float deltaYaw, float deltaPitch)
{
	pCam->yaw += deltaYaw;
	pCam->pitch += deltaPitch;

	if (pCam->pitch > QK_PI_2 * 0.98f)
	{
		pCam->pitch = QK_PI_2 * 0.98f;
	}
	if (pCam->pitch < -QK_PI_2 * 0.98f)
	{
		pCam->pitch = -QK_PI_2 * 0.98f;
	}
}

void qkCameraWorldToScreen(const qkCamera* pCam, const qkVec3* pPoint, float screenWidth, float screenHeight, qkVec3* pOut)
{
	qkVec3 toPoint, right, up, forward;

	qkVec3Sub(pPoint, &pCam->position, &toPoint);
	qkCameraRight(pCam, &right);
	qkCameraUp(pCam, &up);
	qkCameraForward(pCam, &forward);

	float x = qkVec3Dot(&toPoint, &right);
	float y = qkVec3Dot(&toPoint, &up);
	float z = qkVec3Dot(&toPoint, &forward);

	if (z < pCam->nearZ)
	{
		*pOut = qkVec3Create(-1, -1, 0);
		return;
	}

	float aspectRatio = screenWidth / screenHeight;
	float scale		  = tanf(pCam->fov * 0.5f);

	x = (x / (z * scale * aspectRatio) * 0.5f + 0.5f) * screenWidth;
	y = (-y / (z * scale) * 0.5f + 0.5f) * screenHeight;

	*pOut = qkVec3Create(x, y, z);
}