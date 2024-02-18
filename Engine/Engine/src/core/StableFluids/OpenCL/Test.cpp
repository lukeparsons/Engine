#include "../../../opencl/kernel.hpp"

string opencl_c_container()
{
	return R(
		kernel void addsource(global float* grid)
		{
			const uint n = get_global_id(0);
			grid[n] = grid[n] + 1.0f;
		}

		kernel void boundary(global float* grid, global float* grid2)
		{
			const uint n = get_global_id(0);
			grid[n] = -grid[n];
			grid2[n] = grid2[n] + 1.0f;
		}
	);
}
