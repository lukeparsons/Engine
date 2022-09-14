#include "Vector3f.h"
#include <cmath>
#include <functional>
#include <iostream>

Vector3f::Vector3f()
{
	Vector3f(0, 0, 0);
}

Vector3f::Vector3f(float x, float y, float z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

void Vector3f::normalise()
{
	float length = sqrt(x * x + y * y + z * z);

	x /= length;
	y /= length;
	z /= length;
}


Vector3f operator+(const Vector3f& left, const Vector3f& right)
{
	return Vector3f(left.x + right.x, left.y + right.y, left.z + right.z);
}

Vector3f operator+=(Vector3f& left, const Vector3f& right)
{
	left = left + right;
	return left;
}

Vector3f operator-(const Vector3f& left, const Vector3f& right)
{
	return Vector3f(left.x - right.x, left.y - right.y, left.z - right.z);
}

Vector3f operator-=(Vector3f& left, const Vector3f& right)
{
	left = left - right;
	return left;
}

Vector3f operator*(const float scalar, Vector3f& vector)
{
	return Vector3f(vector.x * scalar, vector.y * scalar, vector.z * scalar);
}

Vector3f operator*=(Vector3f& vector, const float scalar)
{
	vector = scalar * vector;
	return vector;
}

Vector3f cross(const Vector3f& left, const Vector3f& right)
{
	float x = left.y * right.z - left.z * right.y;
	float y = left.z * right.x - left.x * right.z;
	float z = left.x * right.y - left.y * right.x;
	return Vector3f(x, y, z);
}

void Print(const Vector3f& vector)
{
	std::cout << vector.x << ", " << vector.y << ", " << vector.z << std::endl;
}