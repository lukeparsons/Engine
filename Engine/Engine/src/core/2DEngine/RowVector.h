#pragma once

/* A row vector is a (row * size) x 1 vector
One row of the vector stores information about one cell in the grid (i, j)
We store a one cell thick halo grid
*/
template<size_t row, size_t column>
class RowVector
{
private:
	std::array<double, row * column + (2 * (row + 2)) + (2 * column)> vector;
public:

	RowVector()
	{
		for(size_t i = 0; i <= column + 1; i++) // Bottom and top halo
		{
			vector[i] = 0;
			vector[i + (column + 2) * (row + 1)] = 0;
		}

		for(size_t j = 1; j <= row; j++) // Left and right halo
		{
			vector[(column + 2) * j] = 0;
			vector[(column + 1) + (column + 2) * j] = 0;
		}
	};

	inline double operator[](size_t idx) const
	{
		return vector[idx];
	}

	inline double& operator[](size_t idx)
	{
		return vector[idx];
	}

	inline double operator()(size_t i, size_t j) const
	{
		return vector[(i + 1) + column * (j + 1)];
	}

	inline double& operator()(size_t i, size_t j)
	{
		return vector[(i + 1) + column * (j + 1)];
	}

	constexpr inline RowVector<row, column> operator*(double scalar) const
	{
		RowVector<row, column> result;
		for(size_t i = 0; i < row * column; i++)
		{
			result[i] = vector[i] * scalar;
		}
		return result;
	}

	constexpr inline RowVector<row, column> operator+(const RowVector<row, column>& rhs) const
	{
		RowVector<row, column> result;
		for(size_t i = 0; i < row * column; i++)
		{
			result[i] = vector[i] + rhs[i];
		}
		return result;
	}

	constexpr inline RowVector<row, column> operator-(const RowVector<row, column>& rhs) const
	{
		RowVector<row, column> result;
		for(size_t i = 0; i < row * column; i++)
		{
			result[i] = vector[i] - rhs[i];
		}
		return result;
	}

	constexpr inline double max()
	{
		return *std::max_element(vector.begin(), vector.end());
	}
};

template<size_t row, size_t column>
constexpr inline RowVector<row, column> operator*(double scalar, const RowVector<row, column>& vector)
{
	return vector * scalar;
}

template<size_t row, size_t column>
inline double DotProduct(const RowVector<row, column>& lhs, const RowVector<row, column>& rhs)
{
	double result = 0;
	for(size_t i = 0; i < row * column; i++)
	{
		result += lhs[i] * rhs[i];
	}
	return result;
}