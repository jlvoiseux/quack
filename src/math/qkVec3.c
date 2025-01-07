#include "quack/math/qkVec3.h"

#include <math.h>

qkVec3 qkVec3Create(float x, float y, float z)
{
	qkVec3 v = {x, y, z};
	return v;
}

void qkVec3Add(const qkVec3* a, const qkVec3* b, qkVec3* out)
{
	out->x = a->x + b->x;
	out->y = a->y + b->y;
	out->z = a->z + b->z;
}

void qkVec3Sub(const qkVec3* a, const qkVec3* b, qkVec3* out)
{
	out->x = a->x - b->x;
	out->y = a->y - b->y;
	out->z = a->z - b->z;
}

void qkVec3Scale(const qkVec3* v, float scalar, qkVec3* out)
{
	out->x = v->x * scalar;
	out->y = v->y * scalar;
	out->z = v->z * scalar;
}

float qkVec3Dot(const qkVec3* a, const qkVec3* b)
{
	return a->x * b->x + a->y * b->y + a->z * b->z;
}

void qkVec3Cross(const qkVec3* a, const qkVec3* b, qkVec3* out)
{
	out->x = a->y * b->z - a->z * b->y;
	out->y = a->z * b->x - a->x * b->z;
	out->z = a->x * b->y - a->y * b->x;
}

float qkVec3Length(const qkVec3* v)
{
	return sqrtf(qkVec3Dot(v, v));
}

void qkVec3Normalize(const qkVec3* v, qkVec3* out)
{
	float len = qkVec3Length(v);
	if (len > 0.0f)
	{
		qkVec3Scale(v, 1.0f / len, out);
	}
	else
	{
		*out = *v;
	}
}