#pragma once
#include <algorithm>
#include <array>

struct GridDataPoint
{

	enum CellState { FLUID, SOLID, EMPTY, DEFAULT } cellState;

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

	GridDataPoint() : cellState(GridDataPoint::EMPTY), Adiag(0), Ax(0), Ay(0) {};
	GridDataPoint(CellState state) : cellState(state), Adiag(0), Ax(0), Ay(0) {};
};

/* This data structure relies on an ordered insertion
* for(1 -> column) { for(1 -> row) { insert() } } is the required order
*/
template<typename T, size_t row, size_t column>
struct GridStructure
{
protected:



public:
	// For coordinates i, j the GridDataPoint for the cell is stored at i + (column + 4) * j
	std::array<T, row* column> grid;

	GridStructure() = default;

	GridStructure(T initValue)
	{
		grid.fill(initValue);
	}

	void fill(T value)
	{
		grid.fill(value);
	}

	const T& max()
	{
		return *std::max_element(grid.begin(), grid.end());
	}

	virtual void insert(T& dataPoint, size_t i, size_t j)
	{
		grid[i + column * j] = dataPoint;
	}

	virtual void insert(T&& dataPoint, size_t i, size_t j)
	{
		grid[i + column * j ] = dataPoint;
	}

	virtual inline T& operator()(size_t i, size_t j)
	{
		return grid[i + column * j];
	}

	virtual const inline T& operator()(size_t i, size_t j) const
	{
		return grid[i + column * j];
	}
};

/*The grid is made up of cells from(2, 2) to(column + 2, row + 2) with a halo(two cell thick wall) around it
* The outer halo bottom left corner is at(0, 0) and the top right is at(column + 3, row + 3)
* The inner halo bottom left corner is at(1, 1) and the top right is at(column + 2, row + 2)
* But we can index from(0, 0) to(column, row) by adding 2 to(i, j) */
template<typename T, size_t row, size_t column>
struct GridStructureHalo : public GridStructure<T, row + 4, column + 4>
{
	// TODO: Possibly redundant
	GridStructureHalo() : GridStructure<T, row + 4, column + 4>() {
		for(size_t i = 0; i <= column + 3; i++) // Bottom and top halo
		{
			// Outer halo
			this->grid[i] = T(); // (0, 0) to (column + 4, 0)
			this->grid[i + (column + 4) * (row + 3)] = T(); // (0, row + 3) to (column + 3, row + 3)

			// Inner halo
			this->grid[i + (column + 4) * 1] = T(); // (0, 1) to (column + 3, 1)
			this->grid[i + (column + 4) * (row + 2)] = T(); // (0, row + 2) to (column + 3, row + 2)
		}

		for(size_t j = 2; j <= row; j++) // Left and right halo
		{
			// Outer halo
			this->grid[(column + 4) * j] = T(); // (0, 2) to (0, row)
			this->grid[(column + 3) + (column + 4) * j] = T(); // (column + 3, 2) to (column + 3, row)

			// Inner halo
			this->grid[1 + (column + 4) * j] = T(); // (1, 2) to (1, row)
			this->grid[(column + 2) + (column + 4) * j] = T(); // (column + 2, 2) to (column + 2, row)
		}
	}

	GridStructureHalo(T initValue) : GridStructure<T, row + 4, column + 4>(initValue) {};

	virtual void insert(T& dataPoint, size_t i, size_t j) override
	{
		this->grid[(i + 2) + (column + 4) * (j + 2)] = dataPoint;
	}

	virtual void insert(T&& dataPoint, size_t i, size_t j) override
	{
		this->grid[(i + 2) + (column + 4) * (j + 2)] = dataPoint;
	}

	virtual inline T& operator()(size_t i, size_t j) override
	{
		return this->grid[(i + 2) + (column + 4) * (j + 2)];
	}

	virtual const inline T& operator()(size_t i, size_t j) const override
	{
		return this->grid[(i + 2) + (column + 4) * (j + 2)];
	}
};

