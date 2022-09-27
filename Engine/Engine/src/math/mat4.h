#pragma once
#include <iostream>
#include "Vector3f.h"
class mat4
{
private:
#pragma warning( suppress: 26495 )
	mat4() {};
public:

	float matrix[4][4];

	mat4(float a11, float a12, float a13, float a14,
		float a21, float a22, float a23, float a24,
		float a31, float a32, float a33, float a34,
		float a41, float a42, float a43, float a44)
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

	const mat4 operator*(const mat4& other) const
	{
		mat4 resultMatrix;
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

	void PrintMat4()
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

mat4 GetTranslationMatrix(const Vector3f& translation);
mat4 GetScaleMatrix(const Vector3f& scale);

mat4 GetXRotationMatrix(float angleRadians, const Vector3f& pivot);
mat4 GetXRotationMatrix(float angleRadians);

mat4 GetYRotationMatrix(float angleRadians, const Vector3f& pivot);
mat4 GetYRotationMatrix(float angleRadians);

mat4 GetZRotationMatrix(float angleRadians, const Vector3f& pivot);
mat4 GetZRotationMatrix(float angleRadians);

mat4 GetProjectionMatrix(float verticalFOV, float horizontalFOV, float aspectRatio);

