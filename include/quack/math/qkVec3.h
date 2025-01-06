#pragma once

struct qkVec3
{
	float x;
	float y;
	float z;

	qkVec3(float x = 0, float y = 0, float z = 0);

	qkVec3 operator+(const qkVec3& other) const;
	qkVec3 operator-(const qkVec3& other) const;
	qkVec3 operator*(float scalar) const;

	float dot(const qkVec3& other) const;
	qkVec3	cross(const qkVec3& other) const;
	float length() const;
	qkVec3	normalized() const;
};