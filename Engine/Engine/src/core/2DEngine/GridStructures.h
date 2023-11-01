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
* for(1 -> column) { for(1 -> row) { insert() } } is the required order
* The grid is made up of cells from (2, 2) to (column + 2 , row + 2) with a halo (two cell thick wall) around it
* The outer halo bottom left corner is at (0, 0) and the top right is at (column + 3, row + 3)
* The inner halo bottom left corner is at (1, 1) and the top right is at (column + 2, row + 2)
* But we can index from (0, 0) to (column, row) by adding 2 to (i, j)
*/
template<typename T, size_t row, size_t column>
struct GridStructure
{
protected:
	// For coordinates i, j the GridDataPoint for the cell is stored at i + (column + 4) * j
	std::array < T, (row + 4) * (column + 4)> grid;

public:

	GridStructure()
	{
		for(size_t i = 0; i <= column + 3; i++) // Bottom and top halo
		{
			// Outer halo
			grid[i] = GridDataPoint(); // (0, 0) to (column + 4, 0)
			grid[i + (column + 4) * (row + 3)] = GridDataPoint(); // (0, row + 3) to (column + 3, row + 3)

			// Inner halo
			grid[i + (column + 4) * 1] = GridDataPoint(); // (0, 1) to (column + 3, 1)
			grid[i + (column + 4) * (row + 2)] = GridDataPoint(); // (0, row + 2) to (column + 3, row + 2)
		}

		for(size_t j = 2; j <= row; j++) // Left and right halo
		{
			// Outer halo
			grid[(column + 4) * j] = GridDataPoint(); // (0, 2) to (0, row)
			grid[(column + 3) + (column + 4) * j] = GridDataPoint(); // (column + 3, 2) to (column + 3, row)

			// Inner halo
			grid[1 + (column + 4) * j] = GridDataPoint(); // (1, 2) to (1, row)
			grid[(column + 2) + (column + 4) * j] = GridDataPoint(); // (column + 2, 2) to (column + 2, row)
		}
	};

	void insert(T& dataPoint, size_t i, size_t j)
	{
		grid[(i + 2) + (column + 4) * (j + 2)] = dataPoint;
	}

	void insert(T&& dataPoint, size_t i, size_t j)
	{
		grid[(i + 2) + (column + 4) * (j + 2)] = dataPoint;
	}

	inline T& operator()(size_t i, size_t j)
	{
		return grid[(i + 2) + (column + 4) * (j + 2)];
	}

	const inline T& operator()(size_t i, size_t j) const
	{
		return grid[(i + 2) + (column + 4) * (j + 2)];
	}

	bool snap_to_grid(int& i, int& j)
	{
		bool snapped = false;
		if(i < 2) // Outside grid on left
		{
			i = 2;
			snapped = true;
		} else if(i > column + 1) // Outside grid on right
		{
			i = column + 1;
			snapped = true;
		}

		if(j < 2) // Outside grid below
		{
			j = 2;
			return true;
		} else if(j > row + 1) // Outside grid above
		{
			j = row + 1;
			return true;
		}
		return snapped;
	}
};

