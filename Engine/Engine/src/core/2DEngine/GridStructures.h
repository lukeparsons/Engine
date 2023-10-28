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
* for(1 -> row) { for(1 -> column) { insert() } } is the required order
* The grid is made up of cells from (1, 1) to (column, row) with a halo (one cell thick wall) around it
*/
template<typename T, size_t row, size_t column>
struct GridStructure
{
protected:
	// For coordinates i, j the GridDataPoint for the cell is stored at i + (column + 2) * j
	std::array < T, row * column + (2 * (row + 2)) + (2 * column)> grid;

public:

	GridStructure()
	{
		for(size_t i = 0; i <= column + 1; i++) // Bottom and top halo
		{
			grid[i] = GridDataPoint();
			grid[i + (column + 2) * (row + 1)] = GridDataPoint();
		}

		for(size_t j = 1; j <= row; j++)// Bottom and top halo
		{
			grid[(column + 2) * j] = GridDataPoint();
			grid[(column + 1) + (column + 2) * j] = GridDataPoint();
		}
	};

	void push_back(T& dataPoint, size_t i, size_t j)
	{
		grid[i + (column + 2) * j] = dataPoint;
	}

	void emplace(T&& dataPoint, size_t i, size_t j)
	{
		grid[i + (column + 2) * j] = dataPoint;
	}

	inline T& operator()(size_t i, size_t j)
	{
		return grid[i + (column + 2) * j];
	}

	const inline T& operator()(size_t i, size_t j) const
	{
		return grid[i + (column + 2) * j];
	}
};

template<size_t row, size_t column>
class RowVector
{
private:
	std::array<double, row * column> vector;
public:

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
		return vector[(i - 1) + column * (j - 1)];
	}

	inline double& operator()(size_t i, size_t j)
	{
		return vector[(i - 1) + column * (j - 1)];
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
		result += lhs[i]* rhs[i];
	}
	return result;
}