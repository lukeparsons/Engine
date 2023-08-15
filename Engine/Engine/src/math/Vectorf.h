#pragma once

struct Vector3f
{
	float x, y, z;

	Vector3f() : x(0), y(0), z(0) {};

	Vector3f(float x, float y, float z) : x(x), y(y), z(z) {};

	Vector3f operator+(const Vector3f& right) const
	{
		return Vector3f(x + right.x, y + right.y, z + right.z);
	}

	Vector3f& operator+=(const Vector3f& right)
	{
		x += right.x;
		y += right.y;
		z += right.z;
		return *this;
	}

	Vector3f operator-(const Vector3f& right) const
	{
		return Vector3f(x - right.x, y - right.y, z - right.z);
	}

	Vector3f& operator-=(const Vector3f& right)
	{
		x -= right.x;
		y -= right.y;
		z -= right.z;
		return *this;
	}

	Vector3f operator*(const float scalar) const
	{
		return Vector3f(x * scalar, y * scalar, z * scalar);
	}

	Vector3f operator*=(const float scalar)
	{
		x *= scalar;
		y *= scalar;
		z *= scalar;
		return *this;
	}
};

Vector3f cross(const Vector3f& left, const Vector3f& right);

Vector3f& normalise(Vector3f& vector);

struct Vector2f
{
	float x, y;

	Vector2f() : x(0), y(0) {};

	Vector2f(float x, float y) : x(x), y(y) {};
};
