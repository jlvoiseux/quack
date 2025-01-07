#pragma once

typedef struct qkVec3
{
	float x;
	float y;
	float z;
} qkVec3;

qkVec3 qkVec3Create(float x, float y, float z);
void   qkVec3Add(const qkVec3* a, const qkVec3* b, qkVec3* out);
void   qkVec3Sub(const qkVec3* a, const qkVec3* b, qkVec3* out);
void   qkVec3Scale(const qkVec3* v, float scalar, qkVec3* out);
float  qkVec3Dot(const qkVec3* a, const qkVec3* b);
void   qkVec3Cross(const qkVec3* a, const qkVec3* b, qkVec3* out);
float  qkVec3Length(const qkVec3* v);
void   qkVec3Normalize(const qkVec3* v, qkVec3* out);