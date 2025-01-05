#include "quack/math/vec3.h"

#include <cmath>

Vec3::Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

Vec3 Vec3::operator+(const Vec3& other) const
{
	return Vec3(x + other.x, y + other.y, z + other.z);
}

Vec3 Vec3::operator-(const Vec3& other) const
{
	return Vec3(x - other.x, y - other.y, z - other.z);
}

Vec3 Vec3::operator*(float scalar) const
{
	return Vec3(x * scalar, y * scalar, z * scalar);
}

float Vec3::dot(const Vec3& other) const
{
	return x * other.x + y * other.y + z * other.z;
}

Vec3 Vec3::cross(const Vec3& other) const
{
	return Vec3(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
}

float Vec3::length() const
{
	return std::sqrt(dot(*this));
}

Vec3 Vec3::normalized() const
{
	float len = length();
	if (len == 0)
		return *this;
	return *this * (1.0f / len);
}