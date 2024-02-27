#pragma once
#include "../../../opencl/opencl.hpp"

inline void test()
{
	Device device(select_device_with_most_flops()); // compile OpenCL C code for the fastest available device
	Memory<float> grid(device, 10);

	cl_command_queue_properties props = CL_QUEUE_ON_DEVICE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_ON_DEVICE_DEFAULT;
	std::cout << props << std::endl;
	cl_int t = 0;
	cl::CommandQueue devQueue = cl::CommandQueue(device.get_cl_context() , device.info.cl_device, props, &t); // queue to push commands for the device
	std::cout << "Command queue " << t << std::endl;
	
	Kernel addsource(device, 10, "addsource", grid, 0.0f);

	for(uint n = 0; n < 10; n++)
	{
		grid[n] = (float)n;
		std::cout << grid[n] << ", ";
	}
	std::cout << std::endl;

	grid.enqueue_write();
	addsource.set_parameters(0, grid, 1.0f);
	addsource.enqueue_run();
	grid.enqueue_read();

	device.get_cl_queue().finish();

	for(uint n = 0; n < 10; n++)
	{
		std::cout << grid[n] << ", ";
	}
	std::cout << std::endl;


}