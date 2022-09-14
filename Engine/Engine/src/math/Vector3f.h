#pragma once

struct Vector3f
{
	Vector3f();
	Vector3f(float x, float y, float z);

	float x, y, z;

	void normalise();
};

Vector3f operator+(const Vector3f& left, const Vector3f& right);
Vector3f operator+=(Vector3f& left, const Vector3f& right);

Vector3f operator-(const Vector3f& left, const Vector3f& right);
Vector3f operator-=(Vector3f& left, const Vector3f& right);

Vector3f operator*(const float scalar, Vector3f& vector);
Vector3f operator*=(Vector3f& vector, const float scalar);

Vector3f cross(const Vector3f& left, const Vector3f& right);

void Print(const Vector3f& vector);