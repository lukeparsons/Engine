#pragma once
#include <algorithm>
#include <array>

class GridDataPoint
{
public:
	Cell2D cell;

	enum CellState { FLUID, SOLID, EMPTY, DEFAULT } cellState;

	float uVelocity; // right arrow of cell
	float vVelocity; // top arrow of cell
	float pressure;

	/* These 'A' named variables store the coefficient matrix for the pressure calculations
	Each row of the matrix corresponds to one fluid cell
	The entries in that row are the coefficients of all the pressure unknowns in the equation for that cell.
	These are the pressure values of the cell's neighbours
	A is symmetric: For example the coefficient of p(i + 1, j, k) in the equation for cell (i, j, k) is stored at
	A(i, j, k),(i + 1, j, k) and must be equal to A(i + 1, j, k)(i, j, k)
	We store three numbers at every grid cell, one for the diagonal entry (i.e, the cell itself), one for the cell to the right and one for the cell directly up
	When we need to refer to an entry like A(i, j)(i - 1, j) we use the symmetry property and instead use A(i - 1, j)(i ,j) = Ax(i - 1, j)
	Thus we only need to store the coefficient for the positive direction in each row */
	float Adiag;  // ADiag stores the coefficient for A(i, j)(i, j)
	float Ax; // Ax stores the coefficient for A(i, j)(i + 1, j)
	float Ay; // Ax stores the coefficient for A(i, j)(i, j + 1)

	double q;
	double z;

	GridDataPoint(Cell2D& _cell, CellState _state) : cell(_cell), cellState(_state),
		uVelocity(0), vVelocity(0), pressure(0), Adiag(0), Ax(0), Ay(0), q(0), z(0) {};

	GridDataPoint(Cell2D&& _cell, CellState _state) : cell(_cell), cellState(_state),
		uVelocity(0), vVelocity(0), pressure(0), Adiag(0), Ax(0), Ay(0), q(0), z(0) {};

	GridDataPoint() : cell(Cell2D()), cellState(EMPTY),
		uVelocity(0), vVelocity(0), pressure(0), Adiag(0), Ax(0), Ay(0), q(0), z(0) {};
};

/* This data structure relies on an ordered insertion
* for(0 -> row) { for(0 -> column) { insert() } } is the required order
*/
template<typename T, size_t row, size_t column>
struct GridStructure
{
protected:
	T defaultObject;
	std::array<T, row * column> grid;
	size_t nextAvailable = 0;

public:

	GridStructure(const T& _defaultObject) : defaultObject(_defaultObject) {};

	void push_back(T& dataPoint)
	{
		grid[nextAvailable] = dataPoint;
		nextAvailable++;
	}

	void emplace(T&& dataPoint)
	{
		grid[nextAvailable] = dataPoint;
		nextAvailable++;
	}

	inline T& operator()(size_t i, size_t j)
	{
		return grid[i * column + j];
	}

	const inline T& operator()(size_t i, size_t j) const
	{
		return grid[i * column + j];
	}

	inline T& at(size_t i, size_t j)
	{
		if(i >= 0 && i < row && j >= 0 && j < row)
		{
			return grid[i * column + j];
		} else
		{
			return defaultObject;
		}
	}

	const inline T& at(size_t i, size_t j) const
	{
		if(i >= 0 && i < row && j >= 0 && j < row)
		{
			return grid[i * column + j];
		} else
		{
			return defaultObject;
		}
	}
};

template<size_t row, size_t column>
class RowVector : public GridStructure<double, row, column>
{
public:

	RowVector() : GridStructure<double, row, column>(0) {};

	constexpr inline RowVector<row, column> operator*(double scalar) const
	{
		RowVector<row, column> result;
		for(size_t i = 0; i < row; i++)
		{
			for(size_t j = 0; j < column; j++)
			{
				result(i, j) = (*this)(i, j) * scalar;
			}
		}
		return result;
	}

	constexpr inline RowVector<row, column> operator*(const RowVector<row, column>& rhs) const
	{
		// TODO: implement
		return rhs;
	}

	constexpr inline RowVector<row, column> operator+(const RowVector<row, column>& rhs) const
	{
		RowVector<row, column> result;
		for(size_t i = 0; i < row; i++)
		{
			for(size_t j = 0; j < column; j++)
			{
				result(i, j) = (*this)(i, j) + rhs(i, j);
			}
		}
		return result;
	}

	constexpr inline RowVector<row, column> operator-(const RowVector<row, column>& rhs) const
	{
		RowVector<row, column> result;
		for(size_t i = 0; i < row; i++)
		{
			for(size_t j = 0; j < column; j++)
			{
				result(i, j) = (*this)(i, j) - rhs(i, j);
			}
		}
		return result;
	}

	constexpr inline double max()
	{
		return *std::max_element(this->grid.begin(), this->grid.end());
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
	for(size_t i = 0; i < row; i++)
	{
		for(size_t j = 0; j < column; j++)
		{
			result += lhs(i, j) * rhs(i, j);
		}
	}
	return result;
}