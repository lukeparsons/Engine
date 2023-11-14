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
template<typename T>
struct GridStructure
{
private:
	size_t column, row;
public:
	// For coordinates i, j the GridDataPoint for the cell is stored at i + (column + 4) * j
	std::vector<T> grid;

	GridStructure(T initValue, size_t _column, size_t _row) : column(_column), row(_row)
	{
		grid = std::vector<T>(row * column);
		std::fill(grid.begin(), grid.end(), initValue);
	}

	void fill(T value)
	{
		std::fill(grid.begin(), grid.end(), value);
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
template<typename T>
struct GridStructureHalo : public GridStructure<T>
{
private:
	// These are the column/row of the usable grid space. The row/column stored in the parent GridStructure class is for the usable space and the halo
	size_t column;
	size_t row;

public:

	GridStructureHalo(T initValue, size_t _column, size_t _row) : GridStructure<T>(initValue, _column + 4, _row + 4), column(_column), row(_row) {};

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
		size_t t = (i + 2) + (column + 4) * (j + 2);
		return this->grid[(i + 2) + (column + 4) * (j + 2)];
	}

	virtual const inline T& operator()(size_t i, size_t j) const override
	{
		return this->grid[(i + 2) + (column + 4) * (j + 2)];
	}
};

