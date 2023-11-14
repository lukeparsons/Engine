#pragma once

/* A row vector is a (row * size) x 1 vector
One row of the vector stores information about one cell in the grid (i, j)
We store a one cell thick halo grid
*/
class RowVector
{
private:
	size_t row, column;
	std::vector<double> vector;
public:

	inline size_t getColumn() const { return column; }

	inline size_t getRow() const { return row; }

	RowVector(size_t _column, size_t _row) : column(_column), row(_row)
	{
		vector = std::vector<double>((column + 2) * (row + 2));
		std::fill(vector.begin(), vector.end(), 0);
	}

	void fill(double val)
	{
		std::fill(vector.begin(), vector.end(), 0);
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

	inline RowVector operator*(double scalar) const
	{
		RowVector result(row, column);
		for(size_t i = 0; i < column; i++)
		{
			for(size_t j = 0; j < row; j++)
			{
				result[i] = (i, j) * scalar;
			}
		}
		return result;
	}

	inline RowVector operator+(const RowVector& rhs) const
	{
		RowVector result(row, column);
		for(size_t i = 0; i < column; i++)
		{
			for(size_t j = 0; j < row; j++)
			{
				result[i] = (i, j) + rhs(i, j);
			}
		}
		return result;
	}

	inline RowVector operator-(const RowVector& rhs) const
	{
		RowVector result(row, column);
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

	std::vector<double>::iterator begin()
	{
		return vector.begin();
	}

	std::vector<double>::iterator end()
	{
		return vector.end();
	}

};

inline RowVector operator*(double scalar, const RowVector& vector)
{
	return vector * scalar;
}

inline double DotProduct(const RowVector& lhs, const RowVector& rhs)
{
	double result = 0;
	for(size_t i = 0; i < lhs.getColumn(); i++)
	{
		for(size_t j = 0; j < lhs.getRow(); j++)
		{
			result += lhs(i, j) * rhs(i, j);
		}
	}
	return result;
}