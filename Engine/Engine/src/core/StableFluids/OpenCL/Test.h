#pragma once
#include "../../../opencl/opencl.hpp"

inline void test()
{
	Device device(select_device_with_most_flops()); // compile OpenCL C code for the fastest available device
	Memory<float> grid(device, 10);

	cl::CommandQueue queue = device.get_cl_queue();
	
	Kernel test(device, 10, 1, "test", grid, 0.0f);

	Kernel test2(device, 10, 1, "test2", grid, 0.0f);

	for(uint n = 0; n < 10; n++)
	{
		grid[n] = (float)n;
		std::cout << grid[n] << ", ";
	}
	std::cout << std::endl;

	cl::Event e;

	grid.enqueue_write();
	test.set_parameters(0, grid, 1.0f);
	std::cout << "Command run 1 " << test.enqueue_run(&e) << std::endl;

	e.wait();

	test2.set_parameters(0, grid, 2.0f);
	std::cout << "Command run 2 " << test2.enqueue_run() << std::endl;

	grid.enqueue_read();

	std::cout << "Exec queue " << queue.finish() << std::endl;

	for(uint n = 0; n < 10; n++)
	{
		std::cout << grid[n] << ", ";
	}
	std::cout << std::endl;


}