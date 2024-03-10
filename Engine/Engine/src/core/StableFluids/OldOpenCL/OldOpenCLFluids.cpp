#include "OldOpenCLFluids.h"
#include <array>
#include <iostream>
#include "../../../opencl/opencl.hpp"

/*TODO:
	- Advection
	- Ink
	- UI
	- Weird workgroup size*/

#define IX(i,j,k) ((i)+(column+2)*(j) + (column+2)*(row+2)*(k)) 

static std::array<int, 3> index_to_coords(uint idx, uint column, uint row)
{
	int k = idx / (column * row);
	int j = (idx - column * row * k) / column;
	int i = (idx - column * row * k) % column;
	return { i, j, k };
}

static Kernel& MakeKernel3D(Kernel&& kernel, uint column, uint row, uint depth)
{
	kernel.set_ranges({ column, row, depth }, { 6, 6, 6 });
	return kernel;
}

static Kernel& MakeKernel2D(Kernel&& kernel, uint x, uint y)
{
	kernel.set_ranges({ x, y }, { 16, 16 });
	return kernel;
}

OldOpenCLFluids::OldOpenCLFluids(unsigned int _column, unsigned int _row, unsigned int _depth, int iterations = 20) : Fluid(_column, _row, _depth, iterations)
{
	Device device(select_device_with_most_flops(), false, get_opencl_c_code_old()); // compile OpenCL C code for the fastest available device

	cl_queue = device.get_cl_queue();

	const uint grid_size = (column + 2) * (row + 2) * (depth + 2); // size of vectors
	const uint centre_grid_size = column * row * depth;

	uVelocity = Memory<float>(device, grid_size); // allocate memory on both host and device
	vVelocity = Memory<float>(device, grid_size);
	wVelocity = Memory<float>(device, grid_size);
	prevUVelocity = Memory<float>(device, grid_size);
	prevVVelocity = Memory<float>(device, grid_size);
	prevWVelocity = Memory<float>(device, grid_size);

	smoke = Memory<float>(device, grid_size);
	prevSmoke = Memory<float>(device, grid_size);

	const float timeStep = 0.4f;
	viscosity = 0.0f;

	const uint workgroup_size = 256u;

	addSource = MakeKernel3D(Kernel(device, grid_size, workgroup_size, "add_source", smoke, prevSmoke, timeStep, column, row), column + 2u, row + 2u, depth + 2u);

	linsolve = MakeKernel3D(Kernel(device, grid_size, workgroup_size, "lin_solve", smoke, prevSmoke, scale, (float)(1 + 6 * scale), column, row), column, row, depth);

	project1 = MakeKernel3D(Kernel(device, grid_size, workgroup_size, "project1", uVelocity, vVelocity, wVelocity, prevUVelocity, prevVVelocity, N, column, row), column, row, depth);
	project2 = MakeKernel3D(Kernel(device, grid_size, workgroup_size, "project2", uVelocity, vVelocity, wVelocity, prevUVelocity, N, column, row), column, row, depth);

	advect = MakeKernel3D(Kernel(device, grid_size, workgroup_size, "advect", smoke, prevSmoke, prevUVelocity, prevVVelocity, prevWVelocity, timeStep, N, column, row, depth), column, row, depth);

	sidesBoundaryFace = MakeKernel2D(Kernel(device, row * depth, workgroup_size, "sidesBoundaryFace", smoke, 1, column, row), row, depth);
	topBottomBoundaryFace = MakeKernel2D(Kernel(device, column * depth, workgroup_size, "topBottomBoundaryFace", smoke, 1, column, row), column, depth);
	frontBackBoundaryFace = MakeKernel2D(Kernel(device, column * row, workgroup_size, "frontBackBoundaryFace", smoke, 1, column, row, depth), column, row);

	boundaryIEdge = Kernel(device, column, workgroup_size, "boundaryIEdge", smoke, column, row, depth);
	boundaryJEdge = Kernel(device, row, workgroup_size, "boundaryJEdge", smoke, column, row, depth);
	boundaryKEdge = Kernel(device, column, workgroup_size, "boundaryKEdge", smoke, column, row, depth);

	boundaryCorners = Kernel(device, 1, 1, "boundaryCorners", smoke, column, row, depth);

	uVelocity.enqueue_fill_device(0.0f);
	vVelocity.enqueue_fill_device(0.0f);
	wVelocity.enqueue_fill_device(0.0f);
	prevUVelocity.enqueue_fill_device(0.0f);
	prevVVelocity.enqueue_fill_device(0.0f);
	prevWVelocity.enqueue_fill_device(0.0f);
	smoke.enqueue_fill_device(0.0f);
	prevSmoke.enqueue_fill_device(0.0f);
}

void OldOpenCLFluids::Simulate(float timeStep, bool& addForceU, bool& addForceV, bool& addForceW, bool& negAddForceU, bool& negAddForceV, bool& negAddForceW, bool& addSmoke, bool& clear)
{

	if(clear)
	{
		uVelocity.enqueue_fill_device(0.0f);
		vVelocity.enqueue_fill_device(0.0f);
		wVelocity.enqueue_fill_device(0.0f);
		prevUVelocity.enqueue_fill_device(0.0f);
		prevVVelocity.enqueue_fill_device(0.0f);
		prevWVelocity.enqueue_fill_device(0.0f);
		smoke.enqueue_fill_device(0.0f);
		prevSmoke.enqueue_fill_device(0.0f);
		clear = false;
	} else
	{
		prevUVelocity.fill_host(0.0f);
		if(addForceU)
		{
			prevUVelocity[IX(column / 2, 2, depth / 2)] = addvel;
			addForceU = false;
		}

		if(negAddForceU)
		{
			prevUVelocity[IX(column / 2, 2, depth / 2)] = -addvel;
			negAddForceU = false;
		}

		prevVVelocity.fill_host(0.0f);
		if(addForceV)
		{
			prevVVelocity[IX(column / 2, 2, depth / 2)] = addvel;
			addForceV = false;
		}

		if(negAddForceV)
		{
			prevVVelocity[IX(column / 2, row / 2, depth / 2)] = -addvel;
			negAddForceV = false;
		}

		prevWVelocity.fill_host(0.0f);
		if(addForceW)
		{
			prevWVelocity[IX(column / 2, row / 2, 2)] = addvel;
			addForceW = false;
		}
		if(negAddForceW)
		{
			prevWVelocity[IX(column / 2, row / 2, depth - 2)] = -addvel;
			negAddForceW = false;
		}

		prevSmoke.fill_host(0.0f);
		if(addSmoke)
		{
			prevSmoke[IX(column / 2, 2, depth / 2)] = addsmoke;
			addSmoke = false;
		}

		prevUVelocity.enqueue_write();
		prevVVelocity.enqueue_write();
		prevWVelocity.enqueue_write();
		prevSmoke.enqueue_write();
	}

	velocity_step(timeStep);
	density_step(timeStep);

	smoke.enqueue_read();

	cl_queue.finish();
}

void OldOpenCLFluids::Profile(float timeStep, float addForceU, float addForceV, float addForceW, float negAddForceU, float negAddForceV, float negAddForceW, float addSmoke)
{

	prevUVelocity.fill_host(0.0f);
	if(addForceU)
	{
		prevUVelocity[IX(column / 2, row / 2, depth / 2)] = addForceU;
	}

	if(negAddForceU)
	{
		prevUVelocity[IX(column / 2, row / 2, depth / 2)] = negAddForceU;
	}

	prevVVelocity.fill_host(0.0f);
	if(addForceV)
	{
		prevVVelocity[IX(column / 2, 2, depth / 2)] = addForceV;
	}

	if(negAddForceV)
	{
		prevVVelocity[IX(column / 2, row / 2, depth / 2)] = negAddForceV;
	}

	prevWVelocity.fill_host(0.0f);
	if(addForceW)
	{
		prevWVelocity[IX(column / 2, row / 2, 2)] = addForceW;
	}
	if(negAddForceW)
	{
		prevWVelocity[IX(column / 2, row / 2, depth - 2)] = -addForceW;
	}

	prevSmoke.fill_host(0.0f);
	if(addSmoke)
	{
		prevSmoke[IX(column / 2, 2, depth / 2)] = addSmoke;
	}

	prevUVelocity.enqueue_write();
	prevVVelocity.enqueue_write();
	prevWVelocity.enqueue_write();
	prevSmoke.enqueue_write();

	velocity_step(timeStep);
	density_step(timeStep);

	smoke.enqueue_read();
	//uVelocity.enqueue_read();
	//vVelocity.enqueue_read();
	//wVelocity.enqueue_read();

	cl_queue.finish();
}

void OldOpenCLFluids::density_step(const float timeStep)
{
	scale = timeStep * diffusionRate * N * N * N;

	addSource.set_parameters(0, smoke, prevSmoke, timeStep);
	addSource.enqueue_run();
	//SWAP(prevSmoke, smoke);

	linsolve.set_parameters(0, prevSmoke, smoke, scale, (float)(1.0f + 6.0f * scale));
	lin_solve(0, prevSmoke);
	//SWAP(prevSmoke, smoke);

	advect.set_parameters(0, smoke, prevSmoke, uVelocity, vVelocity, wVelocity, timeStep);
	advect.enqueue_run(cl::NDRange{ 1, 1, 1 });
	set_boundary(0, smoke);
}

void OldOpenCLFluids::velocity_step(float timeStep)
{
	scale = timeStep * viscosity * N * N * N;

	addSource.set_parameters(0, uVelocity, prevUVelocity, timeStep);
	addSource.enqueue_run();
	addSource.set_parameters(0, vVelocity, prevVVelocity);
	addSource.enqueue_run();
	addSource.set_parameters(0, wVelocity, prevWVelocity);
	addSource.enqueue_run();

	// diffuse (SWAP)

	linsolve.set_parameters(0, prevUVelocity, uVelocity, scale, (float)(1.0f + 6.0f * scale));
	lin_solve(1, prevUVelocity);

	linsolve.set_parameters(0, prevVVelocity, vVelocity);
	lin_solve(2, prevVVelocity);

	linsolve.set_parameters(0, prevWVelocity, wVelocity);
	lin_solve(3, prevWVelocity);

	project(prevUVelocity, prevVVelocity, prevWVelocity, uVelocity, vVelocity);

	// SWAP

	// TODO: only update timestep and dont need to enqueue_run sequentially
	advect.set_parameters(0, uVelocity, prevUVelocity, prevUVelocity, prevVVelocity, prevWVelocity, timeStep);
	advect.enqueue_run(cl::NDRange{ 1, 1, 1 });
	set_boundary(1, uVelocity);

	advect.set_parameters(0, vVelocity, prevVVelocity, prevUVelocity, prevVVelocity, prevWVelocity);
	advect.enqueue_run(cl::NDRange{ 1, 1, 1 });
	set_boundary(2, vVelocity);

	advect.set_parameters(0, wVelocity, prevWVelocity, prevUVelocity, prevVVelocity, prevWVelocity);
	advect.enqueue_run(cl::NDRange{ 1, 1, 1 });
	set_boundary(3, wVelocity);

	project(uVelocity, vVelocity, wVelocity, prevUVelocity, prevVVelocity);

}

void OldOpenCLFluids::project(Memory<float>& u, Memory<float>& v, Memory<float>& w, Memory<float>& p, Memory<float>& div)
{
	project1.set_parameters(0, u, v, w, p, div);
	project1.enqueue_run(cl::NDRange{ 1, 1, 1 });

	set_boundary(0, div);
	set_boundary(0, p);

	linsolve.set_parameters(0, p, div, 1.0f, 6.0f);
	lin_solve(0, p);

	project2.set_parameters(0, u, v, w, p);
	project2.enqueue_run(cl::NDRange{ 1, 1, 1 });

	set_boundary(1, u);
	set_boundary(2, v);
	set_boundary(3, w);
}

void OldOpenCLFluids::lin_solve(const int b, Memory<float>& grid)
{
	for(int t = 0; t < max_iterations; t++)
	{
		linsolve.enqueue_run(cl::NDRange{ 1, 1, 1 });
		set_boundary(b, grid);
	}
}

void OldOpenCLFluids::set_boundary(int b, Memory<float>& grid)
{
	b == 3 ? sidesBoundaryFace.set_parameters(0, grid, -1) : sidesBoundaryFace.set_parameters(0, grid, 1);
	sidesBoundaryFace.enqueue_run(cl::NDRange{ 1, 1 });

	b == 2 ? topBottomBoundaryFace.set_parameters(0, grid, -1) : topBottomBoundaryFace.set_parameters(0, grid, 1);
	topBottomBoundaryFace.enqueue_run(cl::NDRange{ 1, 1 });

	b == 1 ? frontBackBoundaryFace.set_parameters(0, grid, -1) : frontBackBoundaryFace.set_parameters(0, grid, 1);
	frontBackBoundaryFace.enqueue_run(cl::NDRange{ 1, 1 });

	boundaryIEdge.set_parameters(0, grid);
	boundaryIEdge.enqueue_run(cl::NDRange{ 1 });

	boundaryJEdge.set_parameters(0, grid);
	boundaryJEdge.enqueue_run(cl::NDRange{ 1 });

	boundaryKEdge.set_parameters(0, grid);
	boundaryKEdge.enqueue_run(cl::NDRange{ 1 });

	boundaryCorners.set_parameters(0, grid);
	boundaryCorners.enqueue_run(cl::NDRange{ 1 });
}
