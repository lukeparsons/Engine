#include "OpenCLFluids.h"
#include <array>
#include <iostream>

/*TODO:
	- Mouse control
	- UI
	- Ink
	- Different advection
	- Test different workgroup sizes
	- Non-sequential enqueue_running*/

#define IX(i,j,k) ((i)+(column+2)*(j) + (column+2)*(row+2)*(k)) 

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

static Kernel& MakeKernel2D(Kernel&& kernel, uint x, uint y)
{
	kernel.set_ranges({ x, y }, { 1, 1 });
	return kernel;
}

OpenCLFluids::OpenCLFluids(unsigned int _column, unsigned int _row, unsigned int _depth) : column(_column), row(_row), depth(_depth), N(std::max(std::max(column, row), depth))
{
	device = Device(select_device_with_most_flops()); // compile OpenCL C code for the fastest available device
	cl_queue = device.get_cl_queue();
	
	cl_command_queue_properties props = CL_QUEUE_ON_DEVICE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_ON_DEVICE_DEFAULT;
	device_queue = cl::CommandQueue(device.get_cl_context(), props);

	const uint grid_size = (column + 2) * (row + 2) * (depth + 2); // size of vectors

	uVelocity = Memory<float>(device, grid_size); // allocate memory on both host and device
	vVelocity = Memory<float>(device, grid_size);
	wVelocity = Memory<float>(device, grid_size);
	prevUVelocity = Memory<float>(device, grid_size);
	prevVVelocity =  Memory<float>(device, grid_size);
	prevWVelocity = Memory<float>(device, grid_size);

	smoke = Memory<float>(device, grid_size);
	prevSmoke = Memory<float>(device, grid_size);

	const float timeStep = 0.4f;
	viscosity = 0.0f;

	const uint workgroup_size = 64u;

	/*addSource = MakeKernel3D(Kernel(device, grid_size, workgroup_size, "add_source", smoke, prevSmoke, timeStep, column, row), column + 2u, row + 2u, depth + 2u);

	linsolve = MakeKernel3D(Kernel(device, grid_size, workgroup_size, "lin_solve", smoke, prevSmoke, scale, (float)(1 + 6 * scale), column, row), column, row, depth);

	project1 = MakeKernel3D(Kernel(device, grid_size, workgroup_size, "project1", uVelocity, vVelocity, wVelocity, prevUVelocity, prevVVelocity, N, column, row), column, row, depth);
	project2 = MakeKernel3D(Kernel(device, grid_size, workgroup_size, "project2", uVelocity, vVelocity, wVelocity, prevUVelocity, N, column, row), column, row, depth);

	sidesBoundaryFace = MakeKernel2D(Kernel(device, row * depth, workgroup_size, "sidesBoundaryFace", smoke, 1, column, row), row, depth);
	topBottomBoundaryFace = MakeKernel2D(Kernel(device, column * depth, workgroup_size, "topBottomBoundaryFace", smoke, 1, column, row), column, depth);
	frontBackBoundaryFace = MakeKernel2D(Kernel(device, column * row, workgroup_size, "frontBackBoundaryFace", smoke, 1, column, row, depth), column, row);

	boundaryIEdge = Kernel(device, column, workgroup_size, "boundaryIEdge", smoke, column, row, depth);
	boundaryJEdge = Kernel(device, row, workgroup_size, "boundaryJEdge", smoke, column, row, depth);
	boundaryKEdge = Kernel(device, column, workgroup_size, "boundaryKEdge", smoke, column, row, depth);

	boundaryCorners = Kernel(device, 1, 1, "boundaryCorners", smoke, column, row, depth); */

	advect = MakeKernel3D(Kernel(device, grid_size, workgroup_size, "advect", timeStep, smoke, prevSmoke, prevUVelocity, prevVVelocity, prevWVelocity, N, column, row, depth), column, row, depth);

	add_vel_sources = MakeKernel3D(Kernel(device, grid_size, workgroup_size, "add_velocity_sources", 
		uVelocity, vVelocity, wVelocity, prevUVelocity, prevVVelocity, prevWVelocity, timeStep, column, row), column + 2u, row + 2u, depth + 2u);

	add_smoke_source = MakeKernel3D(Kernel(device, grid_size, workgroup_size, "add_source", smoke, prevSmoke, timeStep, column, row), column + 2u, row + 2u, depth + 2u);

	linsolve = MakeKernel3D(Kernel(device, grid_size, workgroup_size, "lin_solve", smoke, prevSmoke, scale, (float)(1 + 6 * scale), column, row), column, row, depth);

	set_boundary = Kernel(device, device_queue, 1, 1, "set_boundary", smoke, 1, 1, 1, column, row, depth);

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

void OpenCLFluids::Simulate(float timeStep, float diffRate, bool& addForceU, bool& addForceV, bool& addForceW, bool& negAddForceU, bool& negAddForceV, bool& negAddForceW, bool& addSmoke, bool& clear,
	float xForce, float yForce)
{

	const float vel = 400.0f;

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
			prevUVelocity[IX(column / 2, 2, depth / 2)] = vel;
			addForceU = false;
		}

		if(negAddForceU)
		{
			prevUVelocity[IX(column - 2, 2, depth / 2)] = -vel;
			negAddForceU = false;
		}

		prevVVelocity.fill_host(0.0f);
		if(addForceV)
		{
			prevVVelocity[IX(column / 2, 2, depth / 2)] = vel;
			addForceV = false;
		}

		if(negAddForceV)
		{
			prevVVelocity[IX(column / 2, row / 2, depth / 2)] = -vel;
			negAddForceV = false;
		}

		prevWVelocity.fill_host(0.0f);
		if(addForceW)
		{
			prevWVelocity[IX(column / 2, row / 2, 2)] = vel;
			addForceW = false;
		}
		if(negAddForceW)
		{
			prevWVelocity[IX(column / 2, row / 2, depth - 2)] = -vel;
			negAddForceW = false;
		}

		prevSmoke.fill_host(0.0f);
		if(addSmoke)
		{
			prevSmoke[IX(column / 2, 2, depth / 2)] = 200.f;
			addSmoke = false;
		}

		prevUVelocity.enqueue_write();
		prevVVelocity.enqueue_write();
		prevWVelocity.enqueue_write();
		prevSmoke.enqueue_write();
	}

	velocity_step(timeStep);
	density_step(timeStep, diffRate);

	smoke.enqueue_read();
	uVelocity.enqueue_read();
	vVelocity.enqueue_read();
	wVelocity.enqueue_read();

	cl_queue.finish();
}

void OpenCLFluids::density_step(const float timeStep, const float diffRate)
{
	scale = timeStep * diffRate * N * N * N;

	cl::Event source_event;
	add_smoke_source.enqueue_run(&source_event);

	source_event.wait();

	//SWAP
	cl::UserEvent bnd_event = cl::UserEvent(device.get_cl_context());
	bnd_event.setStatus(CL_COMPLETE);

	cl::Event linsolve_event;

	linsolve.set_parameters(0, prevSmoke, smoke, scale, (float)(1.0f + 6.0f * scale));
	set_boundary.set_parameters(0, prevSmoke, 1, 1, 1);
	for(int t = 0; t < MAX_ITERATIONS; t++)
	{
		linsolve.enqueue_run(cl::NDRange{ 1, 1, 1 }, {bnd_event}, &linsolve_event);
		set_boundary.enqueue_run({linsolve_event}, &bnd_event);
	}

	//SWAP

	cl::Event advect_event;

	advect.set_parameters(0, timeStep, smoke, prevSmoke, uVelocity, vVelocity, wVelocity);
	advect.enqueue_run(cl::NDRange{ 1, 1, 1 }, &advect_event);
	set_boundary.set_parameters(0, smoke, 1, 1, 1);
	set_boundary.enqueue_run({advect_event});
}

void OpenCLFluids::velocity_step(float timeStep)
{
	scale = timeStep * viscosity * N * N * N;

	cl::Event source_event;
	add_vel_sources.enqueue_run(&source_event);
	 
	source_event.wait();

	// SWAP

	cl::Event solveU_event, solveV_event, solveW_event;
	// todo move create
	cl::UserEvent bndU_event = cl::UserEvent(device.get_cl_context());
	cl::UserEvent bndV_event = cl::UserEvent(device.get_cl_context());
	cl::UserEvent bndW_event = cl::UserEvent(device.get_cl_context());
	bndU_event.setStatus(CL_COMPLETE);
	bndV_event.setStatus(CL_COMPLETE);
	bndW_event.setStatus(CL_COMPLETE);
	
	cl::vector<Event> uEvents, vEvents, wEvents;

	for(int t = 0; t < MAX_ITERATIONS; t++)
	{
		linsolve.set_parameters(0, prevUVelocity, uVelocity, scale, (float)(1.0f + 6.0f * scale));
		linsolve.enqueue_run(cl::NDRange{ 1, 1, 1 }, {bndU_event}, &solveU_event);

		linsolve.set_parameters(0, prevVVelocity, vVelocity);
		{
			linsolve.enqueue_run(cl::NDRange{ 1, 1, 1 }, {bndV_event}, &solveV_event);
		}

		linsolve.set_parameters(0, prevWVelocity, wVelocity);
		linsolve.enqueue_run(cl::NDRange{ 1, 1, 1 }, {bndW_event}, &solveW_event);

		set_boundary.set_parameters(0, prevUVelocity, -1, 1, 1);
		set_boundary.enqueue_run({solveU_event}, &bndU_event);

		set_boundary.set_parameters(0, prevVVelocity, 1, -1, 1);
		set_boundary.enqueue_run({ solveV_event }, &bndV_event);

		set_boundary.set_parameters(0, prevWVelocity, 1, 1, -1);
		set_boundary.enqueue_run({ solveW_event }, &bndW_event);
	}

	cl::Event::waitForEvents({ bndU_event, bndV_event, bndW_event });

	project(prevUVelocity, prevVVelocity, prevWVelocity, uVelocity, vVelocity);

	//SWAP

	cl::Event advect_u, advect_v, advect_w;
	cl::Event uBnd, vBnd, wBnd;

	advect.set_parameters(0, timeStep, uVelocity, prevUVelocity);
	advect.enqueue_run(cl::NDRange{ 1, 1, 1 }, &advect_u);


	advect.set_parameters(0, timeStep, vVelocity, prevVVelocity);
	advect.enqueue_run(cl::NDRange{ 1, 1, 1 }, &advect_v);

	advect.set_parameters(0, timeStep, wVelocity, prevWVelocity);
	advect.enqueue_run(cl::NDRange{ 1, 1, 1 }, &advect_w);

	set_boundary.set_parameters(0, uBnd, -1, 1, 1);
	set_boundary.enqueue_run(&uBnd);
	set_boundary.set_parameters(0, vBnd, 1, -1, 1);
	set_boundary.enqueue_run(&vBnd);
	set_boundary.set_parameters(0, wBnd, 1, 1, -1);
	set_boundary.enqueue_run(&wBnd);

	cl::Event::waitForEvents({ uBnd, vBnd, wBnd });

	project(uVelocity, vVelocity, wVelocity, prevUVelocity, prevVVelocity); 
}

void OpenCLFluids::project(Memory<float>& u, Memory<float>& v, Memory<float>& w, Memory<float>& p, Memory<float>& div)
{
	cl::Event project_event;

	project1.set_parameters(0, u, v, w, p, div);
	project1.enqueue_run(cl::NDRange{ 1, 1, 1 }, &project_event);

	cl::Event div_bnd_event, p_bnd_event;

	set_boundary.set_parameters(0, div, 1, 1, 1);
	set_boundary.enqueue_run({ project_event }, &div_bnd_event);

	set_boundary.set_parameters(0, p, 1, 1, 1);
	set_boundary.enqueue_run({ project_event }, &p_bnd_event);

	cl::Event::waitForEvents({ div_bnd_event, p_bnd_event });

	cl::Event solveP_event;
	cl::UserEvent bnd_event = cl::UserEvent(device.get_cl_context());
	bnd_event.setStatus(CL_COMPLETE);

	linsolve.set_parameters(0, p, div, 1.0f, 6.0f);
	set_boundary.set_parameters(0, p, 1, 1, 1);
	for(int t = 0; t < MAX_ITERATIONS; t++)
	{
		linsolve.enqueue_run(cl::NDRange{1, 1, 1}, {bnd_event}, &solveP_event);
		set_boundary.enqueue_run({solveP_event}, &bnd_event);
	}

	cl::Event project2_event;
	project2.set_parameters(0, u, v, w, p);
	project2.enqueue_run(cl::NDRange{ 1, 1, 1 }, {bnd_event}, &project2_event);

	project2_event.wait();

	cl::Event uBnd, vBnd, wBnd;

	set_boundary.set_parameters(0, u, -1, 1, 1);
	set_boundary.enqueue_run(&uBnd);
	set_boundary.set_parameters(0, v, 1, -1, 1);
	set_boundary.enqueue_run(&vBnd);
	set_boundary.set_parameters(0, w, 1, 1, -1);
	set_boundary.enqueue_run(&wBnd);

	cl::Event::waitForEvents({ uBnd, vBnd, wBnd });
} 

/*
void OpenCLFluids::lin_solve(const int b, Memory<float>& grid)
{
	for(int t = 0; t < MAX_ITERATIONS; t++)
	{
		linsolve.enqueue_run(cl::NDRange{ 1, 1, 1 });
		set_boundary(b, grid);
	}
}

void OpenCLFluids::set_boundary(int b, Memory<float>& grid)
{
	// TODO: non sequential
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
} */
