#pragma once
#include "Matrix.h"
#include "Vector.h"

using Matrix4f = Matrixf<4, 4>;
using VectorMatrix = Matrixf<4, 1>;

Matrix4f GetTranslationMatrix(const Vector3f& translation);
Matrix4f GetScaleMatrix(const Vector3f& scale);
Matrixf<4, 1> ApplyTransformationMatrix(const Matrix4f& transformation, const Vector3f& vector);