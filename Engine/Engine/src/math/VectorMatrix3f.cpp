#include "VectorMatrix3f.h"

VectorMatrix3f::VectorMatrix3f(float x, float y, float z)
{
	this->matrix[0][0] = x;
	this->matrix[0][1] = y;
	this->matrix[0][2] = z;
	this->matrix[0][3] = 1;
}

float& VectorMatrix3f::x()
{
	return this->matrix[0][0];
}

float& VectorMatrix3f::y()
{
	return this->matrix[0][1];
}

float& VectorMatrix3f::z()
{
	return this->matrix[0][2];
}

