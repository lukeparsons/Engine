#pragma once
#include <algorithm>
#include <array>
#include <vector>
#include "../../scene/components/RenderComponent.h"

struct GridDataPoint
{

	enum CellState { FLUID, SOLID, EMPTY, DEFAULT } cellState;
	RenderComponent* render;

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
	float Ak;

	GridDataPoint() : cellState(GridDataPoint::EMPTY), render(nullptr), Adiag(0), Ax(0), Ay(0), Ak(0) {};
	GridDataPoint(CellState state) : cellState(state), render(nullptr), Adiag(0), Ax(0), Ay(0), Ak(0) {};
};

/* This data structure relies on an ordered insertion
* for(1 -> column) { for(1 -> row) { insert() } } is the required order
*/
template<typename T>
struct GridStructure
{
private:
	unsigned int column, row, depth;
public:
	// For coordinates i, j the GridDataPoint for the cell is stored at i + (column + 4) * j
	std::vector<T> grid;

	GridStructure(T initValue, unsigned int _column, unsigned int _row, unsigned int _depth) : column(_column), row(_row), depth(_depth)
	{
		grid = std::vector<T>(row * column * depth);
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

	const T& min()
	{
		return *std::min_element(grid.begin(), grid.end());
	}

	virtual void insert(T& dataPoint, unsigned int i, unsigned int j, unsigned int k)
	{
		grid[i + column * (j + row * k)] = dataPoint;
	}

	virtual void insert(T&& dataPoint, unsigned int i, unsigned int j, unsigned int k)
	{
		grid[i + column * (j + row * k)] = dataPoint;
	}

	virtual inline T& operator()(unsigned int i, unsigned int j, unsigned int k)
	{
		return grid[i + column * (j + row * k)];
	}

	virtual inline const T& operator()(unsigned int i, unsigned int j, unsigned int k) const
	{
		return grid[i + column * (j + row * k)];
	}
};

/*The grid is made up of cells from (2, 2, 2) to (column + 1, row + 1, depth + 1) with a halo(two cell thick wall) around it
* The outer halo bottom left corner is at (0, 0, 0) and the top right is at (column + 3, row + 3, depth + 3)
* The inner halo bottom left corner is at (1, 1, 1) and the top right is at (column + 2, row + 2, depth + 3)
* But we can index from (0, 0) to (column - 1, row - 1, depth - 1) by adding 2 to(i, j, k) */
template<typename T>
struct GridStructureHalo : public GridStructure<T>
{
private:
	// These are the column/row of the usable grid space. The row/column stored in the parent GridStructure class is for the usable space and the halo
	unsigned int column;
	unsigned int row;
	unsigned int depth;
public:

	GridStructureHalo(T initValue, unsigned int _column, unsigned int _row, unsigned int _depth) : GridStructure<T>(initValue, _column + 4, _row + 4, _depth + 4), column(_column), row(_row), depth(_depth) {};

	void initLeftHalo(T val)
	{
		for(unsigned int j = 0; j < row + 4; j++)
		{
			for(unsigned int k = 0; k < depth + 4; k++)
			{
				this->grid[0 + (column + 4) * (j + (row + 4) * k)] = val;
				this->grid[1 + (column + 4) * (j + (row + 4) * k)] = val;
			}
		}
	}

	virtual void insert(T& dataPoint, unsigned int i, unsigned int j, unsigned int k) override
	{
		this->grid[(i + 2) + (column + 4) * ((j + 2) + (row + 4) * (k + 2))] = dataPoint;
	}

	virtual void insert(T&& dataPoint, unsigned int i, unsigned int j, unsigned int k) override
	{
		this->grid[(i + 2) + (column + 4) * ((j + 2) + (row + 4) * (k + 2))] = dataPoint;
	}

	virtual inline T& operator()(unsigned int i, unsigned int j, unsigned int k) override
	{
		return this->grid[(i + 2) + (column + 4) * ((j + 2) + (row + 4) * (k + 2))];
	}

	virtual inline const T& operator()(unsigned int i, unsigned int j, unsigned int k) const override
	{
		return this->grid[(i + 2) + (column + 4) * ((j + 2) + (row + 4) * (k + 2))];
	}
};

