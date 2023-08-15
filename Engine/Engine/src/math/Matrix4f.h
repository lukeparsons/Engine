#pragma once
#include "Vectorf.h"
#include <iostream>

class Matrix4f
{
public:

	float matrix[4][4];

	Matrix4f(float a11 = 1, float a12 = 0, float a13 = 0, float a14 = 0,
		float a21 = 0, float a22 = 1, float a23 = 0, float a24 = 0,
		float a31 = 0, float a32 = 0, float a33 = 1, float a34 = 0,
		float a41 = 0, float a42 = 0, float a43 = 0, float a44 = 1)
	{
		matrix[0][0] = a11; matrix[0][1] = a12; matrix[0][2] = a13; matrix[0][3] = a14;
		matrix[1][0] = a21; matrix[1][1] = a22; matrix[1][2] = a23; matrix[1][3] = a24;
		matrix[2][0] = a31; matrix[2][1] = a32; matrix[2][2] = a33; matrix[2][3] = a34;
		matrix[3][0] = a41; matrix[3][1] = a42; matrix[3][2] = a43; matrix[3][3] = a44;
	}

	float* operator[](size_t index)
	{
		return matrix[index];
	}

	const Matrix4f operator*(const Matrix4f& other) const
	{
		Matrix4f resultMatrix;
		for(int i = 0; i < 4; i++)
		{
			for(int j = 0; j < 4; j++)
			{
				resultMatrix[i][j] = matrix[i][0] * other.matrix[0][j] + 
									 matrix[i][1] * other.matrix[1][j] +
									 matrix[i][2] * other.matrix[2][j] +
									 matrix[i][3] * other.matrix[3][j];
			}
		}
		return resultMatrix;
	}

	void PrintMatrix4f()
	{
		for(int i = 0; i < 4; i++)
		{
			for(int j = 0; j < 4; j++)
			{
				std::cout << matrix[i][j] << " ";
			}
			std::cout << std::endl;
		}
	}
};

Matrix4f GetTranslationMatrix(const Vector3f& translation);
Matrix4f GetScaleMatrix(const Vector3f& scale);

Matrix4f GetXRotationMatrix(float angleRadians, const Vector3f& pivot);
Matrix4f GetXRotationMatrix(float angleRadians);

Matrix4f GetYRotationMatrix(float angleRadians, const Vector3f& pivot);
Matrix4f GetYRotationMatrix(float angleRadians);

Matrix4f GetZRotationMatrix(float angleRadians, const Vector3f& pivot);
Matrix4f GetZRotationMatrix(float angleRadians);

Matrix4f GetProjectionMatrix(float verticalFOV, float horizontalFOV, float aspectRatio);

