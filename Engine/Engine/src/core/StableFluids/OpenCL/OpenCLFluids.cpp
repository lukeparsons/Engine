#include "../../../opencl/opencl.hpp"
#include <array>
#include <iostream>

#define IX(i,j,k,column,row)((i)+(j)*(column)+(k)*(column)*(row))

static std::array<int, 3> index_to_coords(uint idx, uint column, uint row)
{
	int k = idx / (column * row);
	int j = (idx - column * row * k) / column;
	int i = (idx - column * row * k) % column;
	return {i, j, k};
}

void initsim(unsigned int column, unsigned int row, unsigned int depth)
{
	Device device(select_device_with_most_flops()); // compile OpenCL C code for the fastest available device

	const uint N = column * row * depth; // size of vectors

	Memory<float> uVelocity(device, N, 3); // allocate memory on both host and device
	Memory<float> vVelocity(device, N);
	Memory<float> wVelocity(device, N);
	Memory<float> prevUVelocity(device, N);
	Memory<float> prevVVelocity(device, N);
	Memory<float> prevWVelocity(device, N);

	const float timeStep = 0.4f;

	Kernel fluidsim(device, N, 1, "add_source", uVelocity, prevUVelocity, timeStep, column, row);
	fluidsim.set_ranges({ 24, 24, 24 }, { 1, 1, 1 });

	// initialize memory
	for(uint n = 0u; n < N; n++)
	{
		uVelocity[n] = 0.0f;
		vVelocity[n] = 0.0f;
		wVelocity[n] = 0.0f;
		prevUVelocity[n] = (float)n;
		prevVVelocity[n] = 0.0f;
		prevWVelocity[n] = 0.0f;
	}

	int idx = IX(2, 0, 0, column, row);

	print_info("Value before kernel execution: = " + to_string(uVelocity[idx]));

	uVelocity.write_to_device_3d(0, 24, 0, 24, 0, 24, 24, 24, 24);
	prevUVelocity.write_to_device();
	fluidsim.run(); // run add_kernel on the device
	uVelocity.read_from_device_3d(0, 24, 0, 24, 0, 24, 24, 24, 24); // copy data from device memory to host memory

	std::array<int, 3> coords = index_to_coords(uVelocity[idx], column, row);
	print_info("Value after kernel execution: = " + to_string(coords[0]) + ", " + to_string(coords[1]) + ", " + to_string(coords[2]));

	//wait();
}