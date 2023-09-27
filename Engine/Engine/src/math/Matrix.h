#pragma once
#include <array>

template<typename T, size_t row, size_t column>
struct Matrix
{
	std::array<std::array<T, column>, row> matrix;

	std::array<T, column>& operator[](const size_t row)
	{
		return matrix[row];
	}

	Matrix() : matrix(std::array<std::array<T, column>, row> {}) {};

	Matrix(const T(&list)[row * column])
	{
		static_assert(std::is_arithmetic<T>::value);

		for(size_t i = 0; i < row; i++)
		{
			for(size_t j = 0; j < column; j++)
			{
				matrix[i][j] = list[i + j];
			}
		}
	};
};

template<typename T, size_t row1, size_t column1, size_t row2, size_t column2>
Matrix<T, row1, column2> operator*(Matrix<T, row1, column1>& lhs, Matrix<T, row2, column2>& rhs)
{
	static_assert(column1 == row2);

	Matrix<T, row1, column2> result;

	for(size_t i = 0; i < row1; i++)
	{
		for(size_t j = 0; j < column2; j++)
		{
			for(size_t k = 0; k < row2; k++)
			{
				result[i][j] += lhs[i][k] * rhs[k][j];
			}
		}
	}
	return result;
}

template<typename T, size_t row, size_t column>
void PrintMatrix(Matrix<T, row, column>& matrix)
{
	for(int i = 0; i < row; i++)
	{
		for(int j = 0; j < column; j++)
		{
			std::cout << matrix[i][j] << " ";
		}
		std::cout << "\n";
	}
}