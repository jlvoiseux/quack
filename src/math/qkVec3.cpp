#include "quack/math/qkVec3.h"

#include <cmath>

qkVec3::qkVec3(float x, float y, float z) : x(x), y(y), z(z) {}

qkVec3 qkVec3::operator+(const qkVec3& other) const
{
	return qkVec3(x + other.x, y + other.y, z + other.z);
}

qkVec3 qkVec3::operator-(const qkVec3& other) const
{
	return qkVec3(x - other.x, y - other.y, z - other.z);
}

qkVec3 qkVec3::operator*(float scalar) const
{
	return qkVec3(x * scalar, y * scalar, z * scalar);
}

float qkVec3::dot(const qkVec3& other) const
{
	return x * other.x + y * other.y + z * other.z;
}

qkVec3 qkVec3::cross(const qkVec3& other) const
{
	return qkVec3(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
}

float qkVec3::length() const
{
	return std::sqrt(dot(*this));
}

qkVec3 qkVec3::normalized() const
{
	float len = length();
	if (len == 0)
		return *this;
	return *this * (1.0f / len);
}