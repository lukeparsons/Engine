#pragma once
#include <iostream>
#include <algorithm>
#include <array>

template<size_t row, size_t column>
struct Matrixf
{

	float matrix[row][column];

	Matrixf() 
	{
		std::fill(&matrix[0][0], &matrix[0][0] + row * column, 0);
	}

	Matrixf(std::initializer_list<float> values)
	{
		std::fill(&matrix[0][0], &matrix[0][0] + row * column, 0);
		int i = 0;
		for (float v : values)
		{
			matrix[0][i] = v;
			i++;
		}
	}

	float* operator[](size_t index)
	{
		return matrix[index];
	}

};

// Returns in row-major form so GL_TRANSPOSE must be true
template<size_t row, size_t column>
float* GetFlatMatrixf(Matrixf<row, column>& mat)
{
	return mat.matrix[0];
}

template<size_t row1, size_t column1, size_t row2, size_t column2>
Matrixf<row1, column2> operator*(const Matrixf<row1, column1>& lhs, const Matrixf<row2, column2>& rhs)
{
	Matrixf<row1, column2> resultMatrix;

	if (column1 != row2)
	{
		return resultMatrix;
	}

	for (int i = 0; i < row1; i++)
	{
		for (int j = 0; j < column2; j++)
		{
			for (int k = 0; k < row2; k++)
			{
				resultMatrix[i][j] += lhs.matrix[i][k] * rhs.matrix[k][j];
			}
		}
	}

	return resultMatrix;
}

template<size_t row, size_t column>
Matrixf<row, column> operator*(const float& scalar, const Matrixf<row, column>& matrix)
{
	Matrixf<row, column> resultMatrix = matrix;

	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < column; j++)
		{
			resultMatrix[i][j] *= scalar;
		}
	}

	return resultMatrix;
}

template<size_t row, size_t column>
void PrintMatrixf(const Matrixf<row, column>& mat)
{
	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < column; j++)
		{
			std::cout << mat.matrix[i][j] << " ";
		}
		std::cout << "\n";
	}
}