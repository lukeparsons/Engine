#pragma once

/* A row vector is a (row * size) x 1 vector
One row of the vector stores information about one cell in the grid (i, j)
We store a one cell thick halo grid
*/
// TODO: This don't work (templates)
template<size_t row, size_t column>
class RowVector
{
private:
	std::array<double, (row + 2) * (column + 2)> vector;
public:

	RowVector()
	{
		vector.fill(0);
	}

	void fill(double val)
	{
		vector.fill(val);
	}

	inline double operator[](size_t idx) const
	{
		return vector[idx];
	}

	inline double& operator[](size_t idx)
	{
		return vector[idx];
	}

	inline const double& operator()(size_t i, size_t j) const
	{
		return vector[(i + 1) + (column + 2) * (j + 1)];
	}

	inline double& operator()(size_t i, size_t j)
	{
		return vector[(i + 1) + (column + 2) * (j + 1)];
	}

	constexpr inline RowVector<row, column> operator*(double scalar) const
	{
		RowVector<row, column> result;
		for(size_t i = 0; i < column; i++)
		{
			for(size_t j = 0; j < row; j++)
			{
				result[i] = (i, j) * scalar;
			}
		}
		return result;
	}

	constexpr inline RowVector<row, column> operator+(const RowVector<row, column>& rhs) const
	{
		RowVector<row, column> result;
		for(size_t i = 0; i < column; i++)
		{
			for(size_t j = 0; j < row; j++)
			{
				result[i] = (i, j) + rhs(i, j);
			}
		}
		return result;
	}

	constexpr inline RowVector<row, column> operator-(const RowVector<row, column>& rhs) const
	{
		RowVector<row, column> result;
		for(size_t i = 0; i < column; i++)
		{
			for(size_t j = 0; j < row; j++)
			{
				result[i] = (i, j) - rhs(i, j);
			}
		}
		return result;
	}

	constexpr inline double max()
	{
		return *std::max_element(vector.begin(), vector.end());
	}

	std::array<double, (row + 2) * (column + 2) >::iterator begin()
	{
		return vector.begin();
	}

	std::array<double, (row + 2) * (column + 2)>::iterator end()
	{
		return vector.end();
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
	for(size_t i = 0; i < column; i++)
	{
		for(size_t j = 0; j < row; j++)
		{
			result += lhs(i, j) * rhs(i, j);
		}
	}
	return result;
}