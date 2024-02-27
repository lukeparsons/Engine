#include "../../../opencl/kernel.hpp"

string opencl_c_container()
{
	return R(

		kernel void extra(global float* grid, const float f)
		{
			float g = f + 1;
		}

		kernel void addsource(global float* grid, const float t)
		{
			ndrange_t ndrange = ndrange_1D(10);
			size_t n = get_global_id(0);
			enqueue_kernel(get_default_queue(), CLK_ENQUEUE_FLAGS_NO_WAIT, ndrange, ^{size_t n = get_global_id(0); grid[n] = 4.0f;});
			//grid[n] = 5.0f;
		}
	);
}