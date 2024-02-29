#include "../../../opencl/kernel.hpp"

string opencl_c_container()
{
	return R(

		kernel void extra(global float* grid, const float f)
		{
			const uint n = get_global_id(0);
			grid[n] = grid[n] + f;
		}

		void run(global float* grid, const float t)
		{
			ndrange_t ndrange = ndrange_1D(10);


			clk_event_t e;

			enqueue_kernel(get_default_queue(), CLK_ENQUEUE_FLAGS_NO_WAIT, ndrange, 0, NULL, &e,
				^ { size_t n = get_global_id(0); grid[n] = grid[n] + 1.0f; });

			enqueue_kernel(get_default_queue(), CLK_ENQUEUE_FLAGS_NO_WAIT, ndrange, 1, &e, NULL,
				^ { extra(grid, 1.0f); });
		}

		kernel void addsource(global float* grid, const float t)
		{
			run(grid, t);
		}
	);
}