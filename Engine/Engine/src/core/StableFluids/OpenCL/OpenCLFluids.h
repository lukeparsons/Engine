#pragma once
#include "../../../opencl/opencl.hpp"

class OpenCLFluids
{
private:
	unsigned int column, row, depth, N;

	float viscosity;

	void set_boundary(int b, Memory<float>& grid);

	Kernel uAddSource, vAddSource, wAddSource, smokeAddSource;
	Kernel project1, project2, linsolve;
	Kernel smokeLinSolve, uLinSolve, vLinSolve, wLinSolve, projectLinSolve;
	Kernel uAdvect, vAdvect, wAdvect, smokeAdvect;

	Memory<float>* uVelocity;
	Memory<float>* vVelocity;
	Memory<float>* wVelocity;
	Memory<float>* prevUVelocity;
	Memory<float>* prevVVelocity;
	Memory<float>* prevWVelocity;
	Memory<float>* prevSmoke;

	inline void read(Memory<float>* grid, unsigned int column, unsigned int row, unsigned int depth)
	{
		grid->read_from_device_3d(0, column, 0, row, 0, depth, column, row, depth);
	}

	inline void read(std::initializer_list<Memory<float>*> list, unsigned int column, unsigned int row, unsigned int depth)
	{
		for(std::initializer_list<Memory<float>*>::reference elem : list)
		{
			elem->read_from_device_3d(0, column, 0, row, 0, depth, column, row, depth);
		}
	}

	inline void write(Memory<float>* grid, unsigned int column, unsigned int row, unsigned int depth)
	{
		grid->write_to_device_3d(0, column, 0, row, 0, depth, column, row, depth);
	}

	inline void write(std::initializer_list<Memory<float>*> list, unsigned int column, unsigned int row, unsigned int depth)
	{
		for(std::initializer_list<Memory<float>*>::reference elem : list)
		{
			elem->write_to_device_3d(0, column, 0, row, 0, depth, column, row, depth);
		}
	}

	inline void read_all(Memory<float>* grid)
	{
		read(grid, column + 2, row + 2, depth + 2);
	}

	inline void read_all(std::initializer_list<Memory<float>*> list)
	{
		read(list, column + 2, row + 2, depth + 2);
	}

	inline void write_all(Memory<float>* grid)
	{
		write(grid, column + 2, row + 2, depth + 2);
	}

	inline void write_all(std::initializer_list<Memory<float>*> list)
	{
		write(list, column + 2, row + 2, depth + 2);
	}

	inline void write_centre(Memory<float>* grid)
	{
		grid->write_to_device_3d(0, column, 0, row, 0, depth, column, row, depth);
	}

	inline void write_centre(std::initializer_list<Memory<float>*> list)
	{
		write(list, column, row, depth);
	}

	inline void read_centre(Memory<float>* grid)
	{
		grid->read_from_device_3d(0, column, 0, row, 0, depth, column, row, depth);
	}

	inline void read_centre(std::initializer_list<Memory<float>*> list)
	{
		read(list, column, row, depth);
	}

	void project();
	void lin_solve(const int b, Kernel& kernel_linsolve, Memory<float>* grid);

	void density_step();
	void velocity_step();

public:
	Memory<float>* smoke;

	OpenCLFluids(unsigned int _column, unsigned int _row, unsigned int _depth);

	void Simulate();
};