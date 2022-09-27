#pragma once
#include "Matrixf.h"
#include "../Vector3f.h"

using Matrix4f = Matrixf<4, 4>;

Matrix4f GetTranslationMat(const Vector3f& translation);
Matrix4f GetScaleMat(const Vector3f& scale);

Matrix4f GetXRotationMat(float angle, const Vector3f& pivot);
Matrix4f GetXRotationMat(float angle);

Matrix4f GetYRotationMat(float angle, const Vector3f& pivot);
Matrix4f GetYRotationMat(float angle);

Matrix4f GetZRotationMat(float angle, const Vector3f& pivot);
Matrix4f GetZRotationMat(float angle);

Matrix4f GetProjectionMat(float verticalFOV, float horizontalFOV, float aspectRatio);