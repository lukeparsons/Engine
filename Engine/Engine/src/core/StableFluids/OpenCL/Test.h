#pragma once
#include "../../../opencl/opencl.hpp"

inline void test()
{
	Device device(select_device_with_most_flops()); // compile OpenCL C code for the fastest available device
	Memory<float> grid(device, 10);
	
	Kernel addsource(device, 10, 10u, "addsource", grid, 0.0f);

	for(uint n = 0; n < 10; n++)
	{
		grid[n] = (float)n;
		std::cout << grid[n] << ", ";
	}
	std::cout << std::endl;

	grid.enqueue_write();
	addsource.set_parameters(0, grid, 1.0f);
	addsource.enqueue_run();

	addsource.set_parameters(0, grid, 2.0f);
	addsource.enqueue_run();
	grid.enqueue_read();

	device.get_cl_queue().finish();

	for(uint n = 0; n < 10; n++)
	{
		std::cout << grid[n] << ", ";
	}
	std::cout << std::endl;


}