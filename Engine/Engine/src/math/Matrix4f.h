#pragma once
#include "Matrix.h"
#include "Vector.h"

using Matrix4f = Matrixf<4, 4>;
using VectorMatrix = Matrixf<4, 1>;

Matrix4f GetTranslationMatrix(const Vector3f& translation);
Matrix4f GetScaleMatrix(const Vector3f& scale);
Matrix4f GetXRotationMatrix(float angle);
Matrix4f GetYRotationMatrix(float angle);
Matrix4f GetZRotationMatrix(float angle);
Matrix4f GetProjectionMatrix(float verticalFOV, float horizontalFOV, float aspectRatio);
Matrixf<4, 1> ApplyTransformationMatrix(const Matrix4f& transformation, const Vector3f& vector);