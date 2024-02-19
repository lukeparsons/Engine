#include "../../../opencl/kernel.hpp"

string opencl_c_container()
{
	return R(
		kernel void addsource(global float* grid, const float t)
		{
			const uint n = get_global_id(0);
			grid[n] = grid[n] + t;
		}
	);
}