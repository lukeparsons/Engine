#include "OpenCLFluids.h"
#include <array>
#include <iostream>

#define IX(i,j,k) ((i)+(column+2)*(j) + (column+2)*(row+2)*(k))

#define SWAP(x0, x) {Memory<float>* tmp = x0; x0 = x; x = tmp;}

#define MAX_ITERATIONS 10

static std::array<int, 3> index_to_coords(uint idx, uint column, uint row)
{
	int k = idx / (column * row);
	int j = (idx - column * row * k) / column;
	int i = (idx - column * row * k) % column;
	return {i, j, k};
}

static Kernel& MakeKernel3D(Kernel&& kernel, uint column, uint row, uint depth)
{
	kernel.set_ranges({ column, row, depth }, { 1, 1, 1 });
	return kernel;
}

OpenCLFluids::OpenCLFluids(unsigned int _column, unsigned int _row, unsigned int _depth) : column(_column), row(_row), depth(_depth), N(std::max(std::max(column, row), depth))
{
	Device device(select_device_with_most_flops()); // compile OpenCL C code for the fastest available device

	const uint grid_size = (column + 2) * (row + 2) * (depth + 2); // size of vectors
	const uint centre_grid_size = column * row * depth;

	uVelocity = new Memory<float>(device, grid_size, 3); // allocate memory on both host and device
	vVelocity = new Memory<float>(device, grid_size, 3);
	wVelocity = new Memory<float>(device, grid_size, 3);
	prevUVelocity = new Memory<float>(device, grid_size, 3);
	prevVVelocity = new Memory<float>(device, grid_size, 3);
	prevWVelocity = new Memory<float>(device, grid_size, 3);
	smoke = new Memory<float>(device, grid_size, 3);
	prevSmoke = new Memory<float>(device, grid_size, 3);

	const float timeStep = 0.4f;
	const float viscosity = 0.0f;

	smokeAddSource = MakeKernel3D(Kernel(device, grid_size, 1, "add_source", *smoke, *prevSmoke, timeStep, column + 2, row + 2), column + 2, row + 2, depth + 2);
	uAddSource = MakeKernel3D(Kernel(device, grid_size, 1, "add_source", *uVelocity, *prevUVelocity, timeStep, column + 2, row + 2), column + 2, row + 2, depth + 2);
	vAddSource = MakeKernel3D(Kernel(device, grid_size, 1, "add_source", *vVelocity, *prevVVelocity, timeStep, column + 2, row + 2), column + 2, row + 2, depth + 2);
	wAddSource = MakeKernel3D(Kernel(device, grid_size, 1, "add_source", *wVelocity, *prevWVelocity, timeStep, column + 2, row + 2), column + 2, row + 2, depth + 2);

	float scale = timeStep * viscosity * N * N * N;
	smokeLinSolve = MakeKernel3D(Kernel(device, centre_grid_size, 1, "lin_solve", *smoke, *prevSmoke, scale, 1 + 6 * scale, column, row), column, row, depth);
	uLinSolve = MakeKernel3D(Kernel(device, centre_grid_size, 1, "lin_solve", *uVelocity, *prevUVelocity, scale, 1 + 6 * scale, column, row), column, row, depth);
	vLinSolve = MakeKernel3D(Kernel(device, centre_grid_size, 1, "lin_solve", *vVelocity, *prevVVelocity, scale, 1 + 6 * scale, column, row), column, row, depth);
	wLinSolve = MakeKernel3D(Kernel(device, centre_grid_size, 1, "lin_solve", *wVelocity, *prevWVelocity, scale, 1 + 6 * scale, column, row), column, row, depth);
	projectLinSolve = MakeKernel3D(Kernel(device, centre_grid_size, 1, "lin_solve", *prevUVelocity, *prevVVelocity, 1, 6, column, row), column, row, depth);

	project1 = MakeKernel3D(Kernel(device, centre_grid_size, 1, "project1", *uVelocity, *vVelocity, *wVelocity, *prevUVelocity, *prevVVelocity, N, column, row), column, row, depth);
	project2 = MakeKernel3D(Kernel(device, centre_grid_size, 1, "project2", *uVelocity, *vVelocity, *wVelocity, *prevUVelocity, N, column, row), column, row, depth);

	smokeAdvect = MakeKernel3D(Kernel(device, centre_grid_size, 1, "advect", *smoke, *prevSmoke, *uVelocity, *vVelocity, *wVelocity, timeStep, N, column, row), column, row, depth);
	uAdvect = MakeKernel3D(Kernel(device, centre_grid_size, 1, "advect", *uVelocity, *prevUVelocity, *uVelocity, *vVelocity, *wVelocity, timeStep, N, column, row), column, row, depth);
	vAdvect = MakeKernel3D(Kernel(device, centre_grid_size, 1, "advect", *vVelocity, *prevVVelocity, *uVelocity, *vVelocity, *wVelocity, timeStep, N, column, row), column, row, depth);
	wAdvect = MakeKernel3D(Kernel(device, centre_grid_size, 1, "advect", *wVelocity, *prevWVelocity, *uVelocity, *vVelocity, *wVelocity, timeStep, N, column, row), column, row, depth);

	// initialize memory
	for(uint n = 0u; n < grid_size; n++)
	{
		(*uVelocity)[n] = 0.0f;
		(*vVelocity)[n] = 0.0f;
		(*wVelocity)[n] = 0.0f;
		(*prevUVelocity)[n] = 0.0f;
		(*prevVVelocity)[n] = 0.0f;
		(*prevWVelocity)[n] = 0.0f;
		(*smoke)[n] = 0.0f;
		(*prevSmoke)[n] = 0.0f;
	}

	write_all({ uVelocity, vVelocity, wVelocity, prevUVelocity, prevVVelocity, prevWVelocity, smoke, prevSmoke });

	//wait();
}

void OpenCLFluids::Simulate()
{
	for(uint n = 0u; n < column * row * depth; n++)
	{
		(*prevUVelocity)[n] = 0.0f;
		(*prevVVelocity)[n] = 0.0f;
		(*prevWVelocity)[n] = 0.0f;
	}
	density_step();
	velocity_step();
}

void OpenCLFluids::density_step()
{
	smokeAddSource.run();
	read_all(smoke);
	SWAP(prevSmoke, smoke);

	lin_solve(0, smokeLinSolve, smoke);
	SWAP(prevSmoke, smoke);

	smokeAdvect.run();
	read_centre(smoke);
	set_boundary(0, *smoke);
}

void OpenCLFluids::velocity_step()
{
	// TODO: non-in-order enqueue for speed up
	uAddSource.run();
	vAddSource.run();
	wAddSource.run();

	read_all({ uVelocity, vVelocity, wVelocity });

	// diffuse
	SWAP(prevUVelocity, uVelocity);
	lin_solve(1, uLinSolve, uVelocity);
	SWAP(prevVVelocity, vVelocity);
	lin_solve(2, vLinSolve, vVelocity);
	SWAP(prevWVelocity, wVelocity);
	lin_solve(3, wLinSolve, wVelocity);

	project();

	SWAP(prevUVelocity, uVelocity);
	SWAP(prevVVelocity, vVelocity);
	SWAP(prevWVelocity, wVelocity);

	uAdvect.run();
	read_centre(uVelocity);
	set_boundary(1, *uVelocity);
	vAdvect.run();
	read_centre(vVelocity);
	set_boundary(2, *vVelocity);
	wAdvect.run();
	read_centre(wVelocity);
	set_boundary(3, *wVelocity);

	project();
}

void OpenCLFluids::project()
{
	project1.run();
	read_centre({ prevUVelocity, prevVVelocity });
	set_boundary(0, *prevUVelocity);
	set_boundary(0, *prevVVelocity);

	lin_solve(0, projectLinSolve, prevUVelocity);

	project2.run();
	read_centre({ uVelocity, vVelocity, wVelocity });

	set_boundary(1, *uVelocity);
	set_boundary(2, *vVelocity);
	set_boundary(3, *wVelocity);
}

void OpenCLFluids::lin_solve(const int b, Kernel& kernel_linsolve, Memory<float>* grid)
{
	for(int t = 0; t < MAX_ITERATIONS; t++)
	{
		kernel_linsolve.run();
		read_centre(grid);
		set_boundary(b, *grid);
	}
}

void OpenCLFluids::set_boundary(int b, Memory<float>& grid)
{
	int i, j, k;

	for(i = 1; i <= column; i++)
	{
		for(j = 1; j <= row; j++)
		{
			grid[IX(i, j, 0)] = b == 3 ? -grid[IX(i, j, 1)] : grid[IX(i, j, 1)];
			grid[IX(i, j, depth + 1)] = b == 3 ? -grid[IX(i, j, depth)] : grid[IX(i, j, depth)];
		}
	}

	for(j = 1; j <= row; j++)
	{
		for(k = 1; k <= depth; k++)
		{
			grid[IX(0, j, k)] = b == 1 ? -grid[IX(1, j, k)] : grid[IX(1, j, k)];
			grid[IX(column + 1, j, k)] = b == 1 ? -grid[IX(column, j, k)] : grid[IX(column, j, k)];
		}
	}


	for(i = 1; i <= column; i++)
	{
		for(k = 1; k <= depth; k++)
		{
			grid[IX(i, 0, k)] = b == 2 ? -grid[IX(i, 1, k)] : grid[IX(i, 1, k)];
			grid[IX(i, row + 1, k)] = b == 2 ? -grid[IX(i, row, k)] : grid[IX(i, row, k)];
		}
	}

	for(i = 1; i <= column; i++)
	{
		grid[IX(i, 0, 0)] = 0.5f * (grid[IX(i, 1, 0)] + grid[IX(i, 0, 1)]);
		grid[IX(i, row + 1, 0)] = 0.5f * (grid[IX(i, row, 0)] + grid[IX(i, row + 1, 1)]);
		grid[IX(i, 0, depth + 1)] = 0.5f * (grid[IX(i, 0, depth)] + grid[IX(i, 1, depth + 1)]);
		grid[IX(i, row + 1, depth + 1)] = 0.5f * (grid[IX(i, row, depth + 1)] + grid[IX(i, row + 1, depth)]);
	}

	for(j = 1; j <= row; j++)
	{
		grid[IX(0, j, 0)] = 0.5f * (grid[IX(1, j, 0)] + grid[IX(0, j, 1)]);
		grid[IX(column + 1, j, 0)] = 0.5f * (grid[IX(column, j, 0)] + grid[IX(column + 1, j, 1)]);
		grid[IX(0, j, depth + 1)] = 0.5f * (grid[IX(0, j, depth)] + grid[IX(1, j, depth + 1)]);
		grid[IX(column + 1, j, depth + 1)] = 0.5f * (grid[IX(column, j, depth + 1)] + grid[IX(column + 1, j, depth)]);
	}

	for(k = 1; k <= depth; k++)
	{
		grid[IX(0, 0, k)] = 0.5f * (grid[IX(0, 1, k)] + grid[IX(1, 0, k)]);
		grid[IX(0, row + 1, k)] = 0.5f * (grid[IX(0, row, k)] + grid[IX(1, row + 1, k)]);
		grid[IX(column + 1, 0, k)] = 0.5f * (grid[IX(column, 0, k)] + grid[IX(column + 1, 1, k)]);
		grid[IX(column + 1, row + 1, k)] = 0.5f * (grid[IX(column + 1, row, k)] + grid[IX(column, row + 1, k)]);
	}

	grid[IX(0, 0, 0)] = (1.0f / 3.0f) * (grid[IX(1, 0, 0)] + grid[IX(0, 1, 0)] + grid[IX(0, 0, 1)]);
	grid[IX(0, row + 1, 0)] = (1.0f / 3.0f) * (grid[IX(1, row + 1, 0)] + grid[IX(0, row, 0)] + grid[IX(0, row + 1, 1)]);

	grid[IX(column + 1, 0, 0)] = (1.0f / 3.0f) * (grid[IX(column, 0, 0)] + grid[IX(column + 1, 1, 0)] + grid[IX(column + 1, 0, 1)]);
	grid[IX(column + 1, row + 1, 0)] = (1.0f / 3.0f) * (grid[IX(column, row + 1, 0)] + grid[IX(column + 1, row, 0)] + grid[IX(column + 1, row + 1, 1)]);

	grid[IX(0, 0, depth + 1)] = (1.0f / 3.0f) * (grid[IX(1, 0, depth + 1)] + grid[IX(0, 1, depth + 1)] + grid[IX(0, 0, depth)]);
	grid[IX(0, row + 1, depth + 1)] = (1.0f / 3.0f) * (grid[IX(1, row + 1, depth + 1)] + grid[IX(0, row, depth + 1)] + grid[IX(0, row + 1, depth)]);

	grid[IX(column + 1, 0, depth + 1)] = (1.0f / 3.0f) * (grid[IX(column, 0, depth + 1)] + grid[IX(column + 1, 1, depth + 1)] + grid[IX(column + 1, 0, depth)]);
	grid[IX(column + 1, row + 1, depth + 1)] = (1.0f / 3.0f) * (grid[IX(column, row + 1, depth + 1)] + grid[IX(column + 1, row, depth + 1)] + grid[IX(column + 1, row + 1, depth)]);
}