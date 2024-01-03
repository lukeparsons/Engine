#pragma once

/* A row vector is a (row * size) x 1 vector
One row of the vector stores information about one cell in the grid (i, j)
We store a one cell thick halo grid
*/
class RowVector2D
{
private:
	unsigned int row, column;
	std::vector<double> vector;
public:

	inline unsigned int getColumn() const { return column; }

	inline unsigned int getRow() const { return row; }

	RowVector2D(unsigned int _column, unsigned int _row) : column(_column), row(_row)
	{
		vector = std::vector<double>((column + 2) * (row + 2));
		std::fill(vector.begin(), vector.end(), 0);
	}

	void fill(double val)
	{
		std::fill(vector.begin(), vector.end(), 0);
	}

	inline double operator[](unsigned int idx) const
	{
		return vector[idx];
	}

	inline double& operator[](unsigned int idx)
	{
		return vector[idx];
	}

	inline const double& operator()(unsigned int i, unsigned int j) const
	{
		return vector[(i + 1) + (column + 2) * (j + 1)];
	}

	inline double& operator()(unsigned int i, unsigned int j)
	{
		return vector[(i + 1) + (column + 2) * (j + 1)];
	}

	inline RowVector2D operator*(double scalar) const
	{
		RowVector2D result(row, column);
		for(unsigned int i = 0; i < column; i++)
		{
			for(unsigned int j = 0; j < row; j++)
			{
				result(i, j) = (i, j) * scalar;
			}
		}
		return result;
	}

	inline RowVector2D operator+(const RowVector2D& rhs) const
	{
		RowVector2D result(row, column);
		for(unsigned int i = 0; i < column; i++)
		{
			for(unsigned int j = 0; j < row; j++)
			{
				result(i, j) = (i, j) + rhs(i, j);
			}
		}
		return result;
	}

	inline RowVector2D operator-(const RowVector2D& rhs) const
	{
		RowVector2D result(row, column);
		for(unsigned int i = 0; i < column; i++)
		{
			for(unsigned int j = 0; j < row; j++)
			{
				result(i, j) = (i, j) - rhs(i, j);
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

inline RowVector2D operator*(double scalar, const RowVector2D& vector)
{
	return vector * scalar;
}

inline double DotProduct(const RowVector2D& lhs, const RowVector2D& rhs)
{
	double result = 0;
	for(unsigned int i = 0; i < lhs.getColumn(); i++)
	{
		for(unsigned int j = 0; j < lhs.getRow(); j++)
		{
			result += lhs(i, j) * rhs(i, j);
		}
	}
	return result;
}