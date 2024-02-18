#pragma once
#include "../../../opencl/opencl.hpp"

inline void test()
{
	Device device(select_device_with_most_flops()); // compile OpenCL C code for the fastest available device
	Memory<float> grid(device, 10);
	Memory<float> grid2(device, 15);
	Kernel addsource(device, 10, "addsource", grid);
	Kernel boundary(device, 8, "boundary", grid, grid2);

	for(unsigned int n = 0; n < 10; n++)
	{
		grid[n] = (float)n;
		std::cout << grid[n] << ", ";
	}
	for(unsigned int n = 0; n < 15; n++)
	{
		grid2[n] = (float)n;
	}
	std::cout << std::endl;
	grid.write_to_device();
	std::cout << "Running addsource kernel" << std::endl;
	addsource.run();
	grid.read_from_device();
	for(unsigned int n = 0; n < 10; n++)
	{
		std::cout << grid[n] << ", ";
	}
	std::cout << std::endl;



	grid.write_to_device_1d(1, 8);
	grid2.write_to_device();
	std::cout << "Running boundary kernel" << std::endl;
	boundary.run();
	grid.read_from_device_1d(1, 8);
	grid2.read_from_device();
	std::cout << "Grid 1" << std::endl;
	for(unsigned int n = 0; n < 10; n++)
	{
		std::cout << grid[n] << ", ";
	}
	std::cout << std::endl;
	std::cout << "Grid 2" << std::endl;
	for(unsigned int n = 0; n < 15; n++)
	{
		std::cout << grid2[n] << ", ";
	}
	std::cout << std::endl;

}