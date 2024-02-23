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

	uVelocity = Memory<float>(device, grid_size, 3); // allocate memory on both host and device
	vVelocity = Memory<float>(device, grid_size, 3);
	wVelocity = Memory<float>(device, grid_size, 3);
	prevUVelocity = Memory<float>(device, grid_size, 3);
	prevVVelocity =  Memory<float>(device, grid_size, 3);
	prevWVelocity = Memory<float>(device, grid_size, 3);

	smoke = Memory<float>(device, grid_size, 3);
	prevSmoke = Memory<float>(device, grid_size, 3);

	//imageRegion[0] = column + 2; imageRegion[1] = row + 2; imageRegion[2] = depth + 2;
	//outputImage = Image3D(device, cl::ImageFormat(CL_R, CL_RGB), column + 2, column + 2, column + 2);

	const float timeStep = 0.4f;
	viscosity = 0.0f;

	const uint workgroup_size = 64u;

	addSource = MakeKernel3D(Kernel(device, grid_size, workgroup_size, "add_source", smoke, prevSmoke, timeStep, column, row), column + 2u, row + 2u, depth + 2u);

	linsolve = MakeKernel3D(Kernel(device, grid_size, workgroup_size, "lin_solve", smoke, prevSmoke, scale, (float)(1 + 6 * scale), column, row), column + 2u, row + 2u, depth + 2u);

	project1 = MakeKernel3D(Kernel(device, grid_size, workgroup_size, "project1", uVelocity, vVelocity, wVelocity, prevUVelocity, prevVVelocity, N, column, row), column + 2u, row + 2u, depth + 2u);
	project2 = MakeKernel3D(Kernel(device, grid_size, workgroup_size, "project2", uVelocity, vVelocity, wVelocity, prevUVelocity, N, column, row), column + 2u, row + 2u, depth + 2u);

	advect = MakeKernel3D(Kernel(device, grid_size, workgroup_size, "advect", smoke, prevSmoke, prevUVelocity, prevVVelocity, prevWVelocity, timeStep, N, column, row, depth), column + 2u, row + 2u, depth + 2u);

	// initialize memory
	for(uint n = 0u; n < grid_size; n++)
	{
		uVelocity[n] = 0.0f;
		vVelocity[n] = 0.0f;
		wVelocity[n] = 0.0f;
		prevUVelocity[n] = 0.0f;
		prevVVelocity[n] = 0.0f;
		prevWVelocity[n] = 0.0f;
		smoke[n] = 0.0f;
		prevSmoke[n] = 0.0f;
	}
	uVelocity.write_to_device();
	vVelocity.write_to_device();
	wVelocity.write_to_device();
	prevUVelocity.write_to_device();
	prevVVelocity.write_to_device();
	prevWVelocity.write_to_device();
	smoke.write_to_device();
	prevSmoke.write_to_device();
	
	//wait();
}

void OpenCLFluids::Simulate(float timeStep, float diffRate, bool& addForceV, bool& addSmoke)
{

	for(uint n = 0u; n < (column + 2) * (row + 2) * (depth + 2); n++)
	{
		prevUVelocity[n] = 0.0f;
		prevVVelocity[n] = 0.0f;
		prevWVelocity[n] = 0.0f;
		prevSmoke[n] = 0.0f;
	}

	if(addForceV)
	{
		prevVVelocity[IX(column / 2, 2, depth / 2)] = 200.f;
		addForceV = false;
	}

	if(addSmoke)
	{
		prevSmoke[IX(column / 2, 2, depth / 2)] = 200.f;
		addSmoke = false;
	}

	prevUVelocity.write_to_device();
	prevVVelocity.write_to_device();
	prevWVelocity.write_to_device();
	prevSmoke.write_to_device();

	velocity_step(timeStep);
	density_step(timeStep, diffRate);
}

void OpenCLFluids::density_step(const float timeStep, const float diffRate)
{
	scale = timeStep * diffRate * N * N * N;

	addSource.set_parameters(0, smoke, prevSmoke, timeStep);
	addSource.run();
	//SWAP(prevSmoke, smoke);

	linsolve.set_parameters(0, prevSmoke, smoke, scale, (float)(1.0f + 6.0f * scale));
	lin_solve(0, prevSmoke);
	//SWAP(prevSmoke, smoke);

	advect.set_parameters(0, smoke, prevSmoke, uVelocity, vVelocity, wVelocity, timeStep);
	advect.run();
	smoke.read_from_device();
	set_boundary(0, smoke);
	smoke.write_to_device();
}

void OpenCLFluids::velocity_step(float timeStep)
{
	scale = timeStep * viscosity * N * N * N;

	// TODO: non-in-order enqueue for speed up
	addSource.set_parameters(0, uVelocity, prevUVelocity, timeStep);
	addSource.run();
	addSource.set_parameters(0, vVelocity, prevVVelocity, timeStep);
	addSource.run();
	addSource.set_parameters(0, wVelocity, prevWVelocity, timeStep);
	addSource.run();

	// diffuse
	//SWAP(prevUVelocity, uVelocity);
	//SWAP(prevVVelocity, vVelocity);
	//SWAP(prevWVelocity, wVelocity);

	linsolve.set_parameters(0, prevUVelocity, uVelocity, scale, (float)(1.0f + 6.0f * scale));
	lin_solve(1, prevUVelocity);

	linsolve.set_parameters(0, prevVVelocity, vVelocity);
	lin_solve(2, prevVVelocity);

	linsolve.set_parameters(0, prevWVelocity, wVelocity);
	lin_solve(3, prevWVelocity);
	
	project(&prevUVelocity, &prevVVelocity, &prevWVelocity, &uVelocity, &vVelocity);

	//SWAP(prevUVelocity, uVelocity);
	//SWAP(prevVVelocity, vVelocity);
	//SWAP(prevWVelocity, wVelocity);

	// TODO: only update timestep and dont need to run sequentially
	advect.set_parameters(0, uVelocity, prevUVelocity, prevUVelocity, prevVVelocity, prevWVelocity, timeStep);
	advect.run();
	uVelocity.read_from_device();
	set_boundary(1, uVelocity);
	uVelocity.write_to_device();

	advect.set_parameters(0, vVelocity, prevVVelocity, prevUVelocity, prevVVelocity, prevWVelocity, timeStep);
	advect.run();
	vVelocity.read_from_device();
	set_boundary(2, vVelocity);
	vVelocity.write_to_device();

	advect.set_parameters(0, wVelocity, prevWVelocity, prevUVelocity, prevVVelocity, prevWVelocity, timeStep);
	advect.run();
	wVelocity.read_from_device();
	set_boundary(3, wVelocity);
	wVelocity.write_to_device();

	project(&uVelocity, &vVelocity, &wVelocity, &prevUVelocity, &prevVVelocity);

}

// TODO remove pointers
void OpenCLFluids::project(Memory<float>* u, Memory<float>* v, Memory<float>* w, Memory<float>* p, Memory<float>* div)
{
	project1.set_parameters(0, *u, *v, *w, *p, *div);
	project1.run();

	p->read_from_device();
	div->read_from_device();
	set_boundary(0, *div);
	set_boundary(0, *p);
	p->write_to_device();
	div->write_to_device();

	linsolve.set_parameters(0, *p, *div, 1.0f, 6.0f);
	lin_solve(0, *p);

	project2.set_parameters(0, *u, *v, *w, *p);
	project2.run();

	u->read_from_device();
	v->read_from_device();
	w->read_from_device();
	set_boundary(1, *u);
	set_boundary(2, *v);
	set_boundary(3, *w);
	u->write_to_device();
	v->write_to_device();
	w->write_to_device();
} 

void OpenCLFluids::lin_solve(const int b, Memory<float>& grid)
{
	for(int t = 0; t < MAX_ITERATIONS; t++)
	{
		linsolve.run();
		grid.read_from_device();
		set_boundary(b, grid);
		grid.write_to_device();
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
