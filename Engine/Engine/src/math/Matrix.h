#pragma once
#include <iostream>
#include <algorithm>

template<size_t row, size_t column>
struct Matrixf
{
	float matrix[row][column] = {0};

	float* operator[](const int index)
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

template<size_t matrixSize>
struct SquareMatrixf : Matrixf<matrixSize, matrixSize> {};

template<size_t matrixSize>
SquareMatrixf<matrixSize> GetDiagonalMatrixf(float diagValues[matrixSize])
{
	SquareMatrixf<matrixSize> diagMatrix;

	for (int i = 0; i < matrixSize; i++)
	{
		for (int j = 0; j < matrixSize; j++)
		{
			if (i == j)
			{
				diagMatrix[i][j] = diagValues[i];
			} else
			{
				diagMatrix[i][j] = 0;
			}
		}
	}

	return diagMatrix;
}

template<size_t matrixSize>
SquareMatrixf<matrixSize> GetIdentityMatrixf()
{
	float allDiagonalOnes[matrixSize];
	std::fill_n(allDiagonalOnes, matrixSize, 1);
	return GetDiagonalMatrixf<matrixSize>(allDiagonalOnes);
}