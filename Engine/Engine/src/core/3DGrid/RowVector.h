#pragma once

/* A row vector is a (row * size) x 1 vector
One row of the vector stores information about one cell in the grid (i, j)
We store a one cell thick halo grid
*/
class RowVector
{
private:
	unsigned int row, column, depth;
	std::vector<double> vector;
public:

	inline unsigned int getColumn() const { return column; }

	inline unsigned int getRow() const { return row; }

	inline unsigned int getDepth() const { return depth; }

	RowVector(unsigned int _column, unsigned int _row, unsigned int _depth) : column(_column), row(_row), depth(_depth)
	{
		vector = std::vector<double>((column + 2) * (row + 2) * (depth + 2));
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

	inline const double& operator()(unsigned int i, unsigned int j, unsigned int z) const
	{
		return vector[(i + 1) + (column + 2) * ((j + 1) + (row + 2) * (z + 1))];
	}

	inline double& operator()(unsigned int i, unsigned int j, unsigned int z)
	{
		return vector[(i + 1) + (column + 2) * ((j + 1) + (row + 2) * (z + 1))];
	}

	inline RowVector operator*(double scalar) const
	{
		RowVector result(row, column, depth);
		for(unsigned int i = 0; i < column; i++)
		{
			for(unsigned int j = 0; j < row; j++)
			{
				for(unsigned int z = 0; z < row; z++)
				{
					result(i, j, z) = (i, j, z) * scalar;
				}
			}
		}
		return result;
	}

	inline RowVector operator+(const RowVector& rhs) const
	{
		RowVector result(row, column, depth);
		for(unsigned int i = 0; i < column; i++)
		{
			for(unsigned int j = 0; j < row; j++)
			{
				for(unsigned int z = 0; z < depth; z++)
				{
					result(i, j, z) = (i, j, z) + rhs(i, j, z);
				}
			}
		}
		return result;
	}

	inline RowVector operator-(const RowVector& rhs) const
	{
		RowVector result(row, column, depth);
		for(unsigned int i = 0; i < column; i++)
		{
			for(unsigned int j = 0; j < row; j++)
			{
				for(unsigned int z = 0; z < depth; z++)
				{
					result(i, j, z) = (i, j, z) - rhs(i, j, z);
				}
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
	for(unsigned int i = 0; i < lhs.getColumn(); i++)
	{
		for(unsigned int j = 0; j < lhs.getRow(); j++)
		{
			for(unsigned int z = 0; z < lhs.getDepth(); z++)
			{
				result += lhs(i, j, z) * rhs(i, j, z);
			}
		}
	}
	return result;
}