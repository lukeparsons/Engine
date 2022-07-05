#pragma once
#include "Matrixf.h"

template<size_t matrixSize>
struct SquareMatrixf : Matrixf<matrixSize, matrixSize> {};

template<size_t matrixSize>
SquareMatrixf<matrixSize> GetDiagonalMatrixf(const float diagValues[matrixSize])
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
	float allDiagonalOnes[matrixSize] = { 1 };
	std::fill_n(allDiagonalOnes, matrixSize, 1);
	return GetDiagonalMatrixf<matrixSize>(allDiagonalOnes);
}
