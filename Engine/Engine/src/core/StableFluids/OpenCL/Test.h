#pragma once
#include "../../../opencl/opencl.hpp"

inline void test()
{
	Device device(select_device_with_most_flops()); // compile OpenCL C code for the fastest available device
	Memory<float> grid(device, 10);
	Memory<float> grid2(device, 10);

	float t = 2.0f;

	Kernel addsource(device, 10, 10u, "addsource", grid, t);

	for(uint n = 0; n < 10; n++)
	{
		grid[n] = (float)n;
		grid2[n] = (float)0.0f;
		std::cout << grid[n] << ", ";
	}
	std::cout << std::endl;

	grid.write_to_device();
	addsource.run();
	grid.read_from_device();

	for(uint n = 0; n < 10; n++)
	{
		std::cout << grid[n] << ", ";
	}
	std::cout << std::endl;

	addsource.set_parameters(0, grid2, 3.0f);
	grid2.write_to_device();
	addsource.run();
	grid2.read_from_device();

	for(uint n = 0; n < 10; n++)
	{
		std::cout << grid2[n] << ", ";
	}
	std::cout << std::endl;


}