#include "../../../opencl/kernel.hpp"

string main_code()
{
	return R(

		kernel void test2(global float* grid, const float t)
		{
			const uint n = get_global_id(0);

			grid[n] = grid[n] * t;
		}

		kernel void test(global float* grid, const float t)
		{
			const uint n = get_global_id(0);

			grid[n] = grid[n] + t;
		}
	);
}


string actions_code()
{
	return R();
}