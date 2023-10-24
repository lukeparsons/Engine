#pragma once
#include <array>


template<typename T, size_t row, size_t column>
struct Matrix
{
	T matrix[row][column];

	virtual T* operator[](const size_t rowRequest)
	{
		return matrix[rowRequest];
	}

	virtual const T* operator[](const size_t rowRequest) const
	{
		return matrix[rowRequest];
	}

	Matrix() 
	{
		for(size_t i = 0; i < row; i++)
		{
			for(size_t j = 0; j < column; j++)
			{
				matrix[i][j] = 0;
			}
		}
	};

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

template<typename T, size_t row, size_t column>
Matrix<T, row, column> operator*(T scalar, const Matrix<T, row, column>& matrix)
{
	Matrix<T, row, column> result;

	for(size_t i = 0; i < row; i++)
	{
		for(size_t j = 0; j < column; j++)
		{
			result[i][j] = scalar * matrix[i][j];
		}
	}
	return result;
}

template<typename T, size_t row1, size_t column1, size_t row2, size_t column2>
Matrix<T, row1, column2> operator*(const Matrix<T, row1, column1>& lhs, const Matrix<T, row2, column2>& rhs)
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
Matrix<T, row, column> operator-(const Matrix<T, row, column>& lhs, const Matrix<T, row, column>& rhs)
{
	Matrix<T, row, column> result;

	for(size_t i = 0; i < row; i++)
	{
		for(size_t j = 0; j < column; j++)
		{
			result[i][j] = lhs[i][j] - rhs[i][j];
		}
	}
	return result;
}

template<typename T, size_t row, size_t column>
Matrix<T, row, column> operator+(const Matrix<T, row, column>& lhs, const Matrix<T, row, column>& rhs)
{
	Matrix<T, row, column> result;

	for(size_t i = 0; i < row; i++)
	{
		for(size_t j = 0; j < column; j++)
		{
			result[i][j] = lhs[i][j] + rhs[i][j];
		}
	}
	return result;
}

template<typename T, size_t row>
T DotProduct(const Matrix<T, row, 1>& lhs, const Matrix<T, row, 1>& rhs) {
	T result = 0;
	for(size_t i = 0; i < row; i++)
	{
		result += lhs[i][0] * rhs[i][0];
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