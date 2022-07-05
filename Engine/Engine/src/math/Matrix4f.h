#pragma once
#include "Matrixf.h"
#include "VectorMatrix3f.h"

using Matrix4f = Matrixf<4, 4>;

Matrix4f GetTranslationMatrix(const VectorMatrix3f& translation);
Matrix4f GetScaleMatrix(const VectorMatrix3f& scale);
Matrix4f GetXRotationMatrix(float angle);
Matrix4f GetYRotationMatrix(float angle);
Matrix4f GetZRotationMatrix(float angle);
Matrix4f GetProjectionMatrix(float verticalFOV, float horizontalFOV, float aspectRatio);