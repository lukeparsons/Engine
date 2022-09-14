#pragma once
#include "Matrixf.h"
#include "Vector3f.h"

using Matrix4f = Matrixf<4, 4>;

Matrix4f GetTranslationMatrix(const Vector3f& translation);
Matrix4f GetScaleMatrix(const Vector3f& scale);

Matrix4f GetXRotationMatrix(float angle, const Vector3f& pivot);
Matrix4f GetXRotationMatrix(float angle);

Matrix4f GetYRotationMatrix(float angle, const Vector3f& pivot);
Matrix4f GetYRotationMatrix(float angle);

Matrix4f GetZRotationMatrix(float angle, const Vector3f& pivot);
Matrix4f GetZRotationMatrix(float angle);

Matrix4f GetProjectionMatrix(float verticalFOV, float horizontalFOV, float aspectRatio);