#pragma once

struct Vector3f
{
	float x, y, z;

	Vector3f() : Vector3f(0, 0, 0) {};

	Vector3f(float x, float y, float z) : x(x), y(y), z(z) {};

	Vector3f operator+(const Vector3f& right) const
	{
		return Vector3f(x + right.x, y + right.y, z + right.z);
	}
	Vector3f& operator+=(const Vector3f& right)
	{
		*this = *this + right;
		return *this;
	}

	Vector3f operator-(const Vector3f& right) const
	{
		return Vector3f(x - right.x, y - right.y, z - right.z);
	}
	Vector3f& operator-=(const Vector3f& right)
	{
		*this = *this - right;
		return *this;
	}

	Vector3f operator*(const float scalar) const
	{
		return Vector3f(x * scalar, y * scalar, z * scalar);
	}
	Vector3f& operator*=(const float scalar)
	{
		*this = *this * scalar;
	}
};

Vector3f& normalise(Vector3f& vector);

Vector3f cross(const Vector3f& left, const Vector3f& right);

void PrintVector(const Vector3f& vector);