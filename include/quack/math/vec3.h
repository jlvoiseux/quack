#pragma once

struct Vec3
{
	float x;
	float y;
	float z;

	Vec3(float x = 0, float y = 0, float z = 0);

	Vec3 operator+(const Vec3& other) const;
	Vec3 operator-(const Vec3& other) const;
	Vec3 operator*(float scalar) const;

	float dot(const Vec3& other) const;
	Vec3  cross(const Vec3& other) const;
	float length() const;
	Vec3  normalized() const;
};