#include "OpenCLFluids.h"
#include <array>
#include <iostream>

#define IX(i,j,k) ((i)+(column+2)*(j) + (column+2)*(row+2)*(k)) 

static cl::Device cldevice;

static std::array<int, 3> index_to_coords(uint idx, uint column, uint row)
{
	int k = idx / (column * row);
	int j = (idx - column * row * k) / column;
	int i = (idx - column * row * k) % column;
	return { i, j, k };
}

static Kernel& MakeKernel3D(Kernel&& kernel, uint column, uint row, uint depth)
{
	size_t test;
	kernel.cl_kernel.getWorkGroupInfo<size_t>(cldevice, CL_KERNEL_WORK_GROUP_SIZE, &test);
	std::cout << test << std::endl;
	kernel.set_ranges({ column, row, depth }, { 6, 6, 6 });
	return kernel;
}

static Kernel& MakeKernel2D(Kernel&& kernel, uint x, uint y)
{
	kernel.set_ranges({ x, y }, { 16, 16 });
	return kernel;
}

OpenCLFluids::OpenCLFluids(unsigned int _column, unsigned int _row, unsigned int _depth) : Fluid(_column, _row, _depth)
{
	device = Device(select_device_with_most_flops(), true, get_opencl_c_code()); // compile OpenCL C code for the fastest available device
	cldevice = device.get_cl_device();
	cl_queue = device.get_cl_queue();

	const uint grid_size = (column + 2) * (row + 2) * (depth + 2); // size of vectors

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

	sidesBoundaryFace = MakeKernel2D(Kernel(device, row * depth, workgroup_size, "sidesBoundaryFace", smoke, 1, column, row), row, depth);
	topBottomBoundaryFace = MakeKernel2D(Kernel(device, column * depth, workgroup_size, "topBottomBoundaryFace", smoke, 1, column, row), column, depth);
	frontBackBoundaryFace = MakeKernel2D(Kernel(device, column * row, workgroup_size, "frontBackBoundaryFace", smoke, 1, column, row, depth), column, row);

	boundaryIEdge = Kernel(device, column, column, "boundaryIEdge", smoke, column, row, depth);
	boundaryJEdge = Kernel(device, row, row, "boundaryJEdge", smoke, column, row, depth);
	boundaryKEdge = Kernel(device, column, depth, "boundaryKEdge", smoke, column, row, depth);

	boundaryCorners = Kernel(device, 1, 1, "boundaryCorners", smoke, column, row, depth);

	advect = MakeKernel3D(Kernel(device, grid_size, workgroup_size, "advect", timeStep, smoke, prevSmoke, prevUVelocity, prevVVelocity, prevWVelocity, N, column, row, depth), column, row, depth);

	add_vel_sources = MakeKernel3D(Kernel(device, grid_size, workgroup_size, "add_velocity_sources",
		timeStep, uVelocity, vVelocity, wVelocity, prevUVelocity, prevVVelocity, prevWVelocity, column, row), column + 2u, row + 2u, depth + 2u);

	add_smoke_source = MakeKernel3D(Kernel(device, grid_size, 1, "add_source", timeStep, smoke, prevSmoke, column, row), column + 2u, row + 2u, depth + 2u);

	linsolve = MakeKernel3D(Kernel(device, grid_size, workgroup_size, "lin_solve", smoke, prevSmoke, scale, (float)(1 + 6 * scale), column, row), column, row, depth);

	project1 = MakeKernel3D(Kernel(device, grid_size, workgroup_size, "project1", uVelocity, vVelocity, wVelocity, prevUVelocity, prevVVelocity, N, column, row), column, row, depth);
	project2 = MakeKernel3D(Kernel(device, grid_size, workgroup_size, "project2", uVelocity, vVelocity, wVelocity, prevUVelocity, N, column, row), column, row, depth);

	uVelocity.enqueue_fill_device(0.0f);
	vVelocity.enqueue_fill_device(0.0f);
	wVelocity.enqueue_fill_device(0.0f);
	prevUVelocity.enqueue_fill_device(0.0f);
	prevVVelocity.enqueue_fill_device(0.0f);
	prevWVelocity.enqueue_fill_device(0.0f);
	smoke.enqueue_fill_device(0.0f);
	prevSmoke.enqueue_fill_device(0.0f);
}

void OpenCLFluids::Simulate(float timeStep, bool& addForceU, bool& addForceV, bool& addForceW, bool& negAddForceU, bool& negAddForceV, bool& negAddForceW, bool& addSmoke, bool& clear)
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
			prevUVelocity[IX(column / 2, row / 2, depth / 2)] = addvel;
			addForceU = false;
		}

		if(negAddForceU)
		{
			prevUVelocity[IX(column / 2, row / 2, depth / 2)] = -addvel;
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
	//uVelocity.enqueue_read();
	//vVelocity.enqueue_read();
	//wVelocity.enqueue_read();

	cl_queue.finish();
}

void OpenCLFluids::Profile(float timeStep, float addForceU, float addForceV, float addForceW, float negAddForceU, float negAddForceV, float negAddForceW, float addSmoke)
{

	prevUVelocity.fill_host(0.0f);

	prevUVelocity[IX(column / 2, row / 2, depth / 2)] = addForceU;

	prevUVelocity[IX(column / 2, row / 2, depth / 2)] = negAddForceU;
	
	prevVVelocity.fill_host(0.0f);

	prevVVelocity[IX(column / 2, 2, depth / 2)] = addForceV;

	prevVVelocity[IX(column / 2, row / 2, depth / 2)] = negAddForceV;

	prevWVelocity.fill_host(0.0f);

	prevWVelocity[IX(column / 2, row / 2, 2)] = addForceW;
	
	prevWVelocity[IX(column / 2, row / 2, depth - 2)] = -addForceW;

	prevSmoke.fill_host(0.0f);

	prevSmoke[IX(column / 2, 2, depth / 2)] = addSmoke;

	prevUVelocity.enqueue_write();
	prevVVelocity.enqueue_write();
	prevWVelocity.enqueue_write();
	prevSmoke.enqueue_write();

	velocity_step(timeStep);
	density_step(timeStep);

	smoke.enqueue_read();

	cl_queue.finish();
}

void OpenCLFluids::density_step(const float timeStep)
{
	scale = timeStep * diffusionRate * N * N * N;
	add_smoke_source.set_parameters(0, timeStep);
	add_smoke_source.enqueue_run(&source_event);

	//SWAP

	bnd_event = { source_event };

	linsolve.set_parameters(0, prevSmoke, smoke, scale, (float)(1.0f + 6.0f * scale));
	for(int t = 0; t < max_iterations; t++)
	{
		linsolve.enqueue_run(cl::NDRange{ 1, 1, 1 }, bnd_event, &linsolve_event);
		bnd_event = enqueue_set_boundary(prevSmoke, 1, 1, 1, { linsolve_event });
	}

	//SWAP

	advect.set_parameters(0, timeStep, smoke, prevSmoke, uVelocity, vVelocity, wVelocity);
	advect.enqueue_run(cl::NDRange{ 1, 1, 1 }, bnd_event, &advect_event);
	enqueue_set_boundary(smoke, 1, 1, 1, { advect_event }); 
}

void OpenCLFluids::velocity_step(float timeStep)
{
	scale = timeStep * viscosity * N * N * N;

	add_vel_sources.set_parameters(0, timeStep);
	add_vel_sources.enqueue_run(&source_event);

	// SWAP

	uBnd = { source_event }, vBnd = { source_event }, wBnd = { source_event };

	for(int t = 0; t < max_iterations; t++)
	{
		linsolve.set_parameters(0, prevUVelocity, uVelocity, scale, (float)(1.0f + 6.0f * scale));
		linsolve.enqueue_run(cl::NDRange{ 1, 1, 1 }, uBnd, &solveU_event);

		linsolve.set_parameters(0, prevVVelocity, vVelocity);
		linsolve.enqueue_run(cl::NDRange{ 1, 1, 1 }, vBnd, &solveV_event);

		linsolve.set_parameters(0, prevWVelocity, wVelocity);
		linsolve.enqueue_run(cl::NDRange{ 1, 1, 1 }, wBnd, &solveW_event);

		uBnd = enqueue_set_boundary(prevUVelocity, -1, 1, 1, { solveU_event });
		vBnd = enqueue_set_boundary(prevVVelocity, 1, -1, 1, { solveV_event });
		wBnd = enqueue_set_boundary(prevWVelocity, 1, 1, -1, { solveW_event });
	}

	cl::WaitForEvents(uBnd);
	cl::WaitForEvents(vBnd);
	cl::WaitForEvents(wBnd);

	project(prevUVelocity, prevVVelocity, prevWVelocity, uVelocity, vVelocity);

	//SWAP

	advect.set_parameters(0, timeStep, uVelocity, prevUVelocity, prevUVelocity, prevVVelocity, prevWVelocity);
	advect.enqueue_run(cl::NDRange{ 1, 1, 1 }, &advect_u);

	advect.set_parameters(0, timeStep, vVelocity, prevVVelocity, prevUVelocity, prevVVelocity, prevWVelocity);
	advect.enqueue_run(cl::NDRange{ 1, 1, 1 }, &advect_v);

	advect.set_parameters(0, timeStep, wVelocity, prevWVelocity, prevUVelocity, prevVVelocity, prevWVelocity);
	advect.enqueue_run(cl::NDRange{ 1, 1, 1 }, &advect_w);

	advect_bnd_u = enqueue_set_boundary(uVelocity, -1, 1, 1, { advect_u });
	advect_bnd_v = enqueue_set_boundary(vVelocity, 1, -1, 1, { advect_v });
	advect_bnd_w = enqueue_set_boundary(wVelocity, 1, 1, -1, { advect_w });

	cl::WaitForEvents(advect_bnd_u);
	cl::WaitForEvents(advect_bnd_v);
	cl::WaitForEvents(advect_bnd_w);

	project(uVelocity, vVelocity, wVelocity, prevUVelocity, prevVVelocity); 
}

void OpenCLFluids::project(Memory<float>& u, Memory<float>& v, Memory<float>& w, Memory<float>& p, Memory<float>& div)
{

	project1.set_parameters(0, u, v, w, p, div);
	project1.enqueue_run(cl::NDRange{ 1, 1, 1 }, &project_event);

	div_bnd_event = enqueue_set_boundary(div, 1, 1, 1, { project_event });
	p_bnd_event = enqueue_set_boundary(p, 1, 1, 1, { project_event });

	cl::WaitForEvents(div_bnd_event);
	cl::WaitForEvents(p_bnd_event);

	bnd_event = { };

	linsolve.set_parameters(0, p, div, 1.0f, 6.0f);
	for(int t = 0; t < max_iterations; t++)
	{
		linsolve.enqueue_run(cl::NDRange{ 1, 1, 1 }, bnd_event, &solveP_event);
		bnd_event = enqueue_set_boundary(p, 1, 1, 1, {solveP_event});
	}

	project2.set_parameters(0, u, v, w, p);
	project2.enqueue_run(cl::NDRange{ 1, 1, 1 }, bnd_event, &project2_event);

	uBnd = enqueue_set_boundary(prevUVelocity, -1, 1, 1, { project2_event });
	vBnd = enqueue_set_boundary(prevVVelocity, 1, -1, 1, { project2_event });
	wBnd = enqueue_set_boundary(prevWVelocity, 1, 1, -1, { project2_event });

	cl::WaitForEvents(uBnd);
	cl::WaitForEvents(vBnd);
	cl::WaitForEvents(wBnd);
}

cl::vector<Event> OpenCLFluids::enqueue_set_boundary(Memory<float>& grid, const int f1, const int f2, const int f3, const cl::vector<Event>& waitEvent)
{

	frontBackBoundaryFace.set_parameters(0, grid, f3);
	frontBackBoundaryFace.enqueue_run(cl::NDRange{ 1, 1, 1 }, waitEvent, &frontBack);

	topBottomBoundaryFace.set_parameters(0, grid, f2);
	topBottomBoundaryFace.enqueue_run(cl::NDRange{ 1, 1, 1 }, waitEvent, &topBottom);

	sidesBoundaryFace.set_parameters(0, grid, f1);
	sidesBoundaryFace.enqueue_run(cl::NDRange{ 1, 1, 1 }, waitEvent, &sidesBoundary);

	boundaryIEdge.set_parameters(0, grid);
	boundaryIEdge.enqueue_run(cl::NDRange{ 1, 1, }, waitEvent, &boundaryI);

	boundaryJEdge.set_parameters(0, grid);
	boundaryJEdge.enqueue_run(cl::NDRange{ 1, 1, }, waitEvent, &boundaryJ);

	boundaryKEdge.set_parameters(0, grid);
	boundaryKEdge.enqueue_run(cl::NDRange{ 1, 1, }, waitEvent, &boundaryK);

	boundaryCorners.set_parameters(0, grid);
	boundaryCorners.enqueue_run(cl::NDRange{ 1 }, waitEvent, &boundaryCorner);

	return { frontBack, topBottom, sidesBoundary, boundaryI, boundaryJ, boundaryK, boundaryCorner };
}