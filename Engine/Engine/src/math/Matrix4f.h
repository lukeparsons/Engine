#pragma once
#include "Matrix.h"
#include "Vector.h"

using Matrix4f = SquareMatrixf<4>;

Matrix4f GetTranslationMatrix(const Vector3f translation);
Matrix4f GetScaleMatrix(const Vector3f scale);