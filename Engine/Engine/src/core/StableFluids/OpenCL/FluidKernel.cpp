#include "../../../opencl/kernel.hpp"

string opencl_c_container()
{
	return R( // ########################## begin of OpenCL C code ####################################################################


	clk_event_t set_boundary(const int b, global float* grid, const uint column, const uint row, const uint depth, const uint blocking_count, clk_event_t* blocking_event)
	{
		queue_t queue = get_default_queue();

		const size_t ones[2] = { 1, 1 };
		const size_t rd[2] = { row, depth };
		const size_t cd[2] = { column, depth };
		const size_t cr[2] = { column, row };

		ndrange_t sidesRange = ndrange_2D(ones, rd, ones);
		ndrange_t topBottomRange = ndrange_2D(ones, cd, ones);
		ndrange_t frontBackRange = ndrange_2D(ones, cr, ones);
		
		if(b == 3)
		{
			enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, sidesRange, blocking_count, blocking_event, NULL, ^{sidesBoundaryFace(grid, -1, column, row);});
		} else
		{
			enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, sidesRange, blocking_count, blocking_event, NULL, ^{sidesBoundaryFace(grid, 1, column, row);});
		}

		if(b == 2)
		{
			enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, topBottomRange, blocking_count, blocking_event, NULL, ^{topBottomBoundaryFace(grid, -1, column, row);});
		} else
		{
			enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, topBottomRange, blocking_count, blocking_event, NULL, ^{topBottomBoundaryFace(grid, 1, column, row);});
		}

		if(b == 1)
		{
			enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, frontBackRange, blocking_count, blocking_event, NULL, ^{frontBackBoundaryFace(grid, -1, column, row, depth);});
		} else
		{
			enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, frontBackRange, blocking_count, blocking_event, NULL, ^{frontBackBoundaryFace(grid, 1, column, row, depth);});
		}

		enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, ndrange_1D(1, column, 1), blocking_count, blocking_event, NULL, ^{boundaryIEdge(grid, column, row, depth);});
		enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, ndrange_1D(1, row, 1), blocking_count, blocking_event, NULL, ^{boundaryJEdge(grid, column, row, depth);});
		enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, ndrange_1D(1, depth, 1), blocking_count, blocking_event, NULL, ^{boundaryKEdge(grid, column, row);});

		clk_event_t exit_event;
		enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, ndrange_1D(1, 1, 1), blocking_count, blocking_event, &exit_event, ^{boundaryCorners(grid, column, row, depth);});
		return exit_event;
	}

	clk_event_t* project(global float* uVelocity, global float* vVelocity, global float* wVelocity, global float* p, global float* div, const uint N, 
		const uint column, const uint row, const uint depth, clk_event_t* wait_event)
	{
		queue_t queue = get_default_queue();

		const size_t ones[3] = { 1, 1, 1 };
		const size_t gridrange[3] = { column, row, depth };
		ndrange_t noBoundaryRange = ndrange_3D(ones, gridrange, ones);

		clk_event_t blocking_event;
		enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, noBoundaryRange, 3, wait_event, &blocking_event, 
			^{project1(uVelocity, vVelocity, wVelocity, p, div, N, column, row);});

		clk_event_t boundary_event[2];
		boundary_event[0] = set_boundary(0, div, column, row, depth, 1, &blocking_event);
		boundary_event[1] = set_boundary(0, p, column, row, depth, 1, &blocking_event);
		 
		int i = 0;
		clk_event_t iter_event;
		for(i = 0; i < MAX_ITERATIONS(); i++)
		{
			enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, noBoundaryRange, 2, boundary_event, &iter_event, ^{lin_solve(p, div, 1.0f, 6.0f, column, row);});
			boundary_event[0] = set_boundary(0, p, column, row, depth, 1, &iter_event);
		}

		enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, noBoundaryRange, 2, boundary_event, &blocking_event, ^{project2(uVelocity, vVelocity, wVelocity, p, N, column, row);});

		clk_event_t boundary_event3[3];
		boundary_event3[0] = set_boundary(1, uVelocity, column, row, depth, 1, &blocking_event);
		boundary_event3[1] = set_boundary(2, vVelocity, column, row, depth, 1, &blocking_event);
		boundary_event3[2] = set_boundary(3, wVelocity, column, row, depth, 1, &blocking_event);
		return boundary_event3;
	}

	kernel void density_step(const float scale, global float* smoke, global float* prevSmoke, global float* uVelocity, global float* vVelocity, global float* wVelocity, const float timeStep,
		const uint N, const uint column, const uint row, const uint depth)
	{
		queue_t queue = get_default_queue();

		const size_t ones[3] = { 1, 1, 1 };
		const size_t source_gridrange[3] = { column + 2, row + 2, depth + 2 };
		ndrange_t addSourceRange = ndrange_3D(source_gridrange, ones);

		clk_event_t source_event;
		enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, addSourceRange, 0, NULL, &source_event, ^{add_source(smoke, prevSmoke, timeStep, column, row);});

		const size_t gridrange[3] = { column, row, depth };
		ndrange_t noBoundaryRange = ndrange_3D(ones, gridrange, ones);

		clk_event_t iter_event;
		int i = 0;
		for(i = 0; i < MAX_ITERATIONS(); i++)
		{
			enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, noBoundaryRange, 1, &source_event, &iter_event, ^{lin_solve(prevSmoke, smoke, scale, 1.0f + 6.0f * scale, column, row);});
			source_event = set_boundary(0, prevSmoke, column, row, depth, 1, &iter_event);
		}

		enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, noBoundaryRange, 1, &source_event, &iter_event, ^{advect(smoke, prevSmoke, uVelocity, vVelocity, wVelocity, timeStep, N, column, row, depth);});
		set_boundary(0, smoke, column, row, depth, 1, &iter_event);
	}

	kernel void velocity_step(const float scale, global float* uVelocity, global float* vVelocity, global float* wVelocity, 
		global float* prevUVelocity, global float* prevVVelocity, global float* prevWVelocity,
		const float timeStep, const uint N, const uint column, const uint row, const uint depth)
	{
		queue_t queue = get_default_queue();

		const size_t ones[3] = { 1, 1, 1 };
		const size_t source_gridrange[3] = { column + 2, row + 2, depth + 2 };
		ndrange_t addSourceRange = ndrange_3D(source_gridrange, ones);

		clk_event_t source_event[3];

		enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, addSourceRange, 0, NULL, &source_event[0], ^{add_source(uVelocity, prevUVelocity, timeStep, column, row);});
		enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, addSourceRange, 0, NULL, &source_event[1], ^{add_source(vVelocity, prevVVelocity, timeStep, column, row);});
		enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, addSourceRange, 0, NULL, &source_event[2], ^{add_source(wVelocity, prevWVelocity, timeStep, column, row);});

		const size_t gridrange[3] = { column, row, depth };
		ndrange_t noBoundaryRange = ndrange_3D(ones, gridrange, ones);

		clk_event_t iter_event[3];
		clk_event_t boundary_event[3];
		enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, noBoundaryRange, 3, source_event, &iter_event[0], ^{lin_solve(prevUVelocity, uVelocity, scale, 1.0f + 6.0f * scale, column, row);});
		enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, noBoundaryRange, 3, source_event, &iter_event[1], ^{lin_solve(prevVVelocity, vVelocity, scale, 1.0f + 6.0f * scale, column, row);});
		enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, noBoundaryRange, 3, source_event, &iter_event[2], ^{lin_solve(prevWVelocity, wVelocity, scale, 1.0f + 6.0f * scale, column, row);});
		boundary_event[0] = set_boundary(1, prevUVelocity, column, row, depth, 1, &iter_event[0]);
		boundary_event[1] = set_boundary(2, prevVVelocity, column, row, depth, 1, &iter_event[1]);
		boundary_event[2] = set_boundary(3, prevWVelocity, column, row, depth, 1, &iter_event[2]);

		int i = 0;
		for(i = 0; i < MAX_ITERATIONS() - 1; i++)
		{
			enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, noBoundaryRange, 3, &boundary_event[0], &iter_event[0], ^{lin_solve(prevUVelocity, uVelocity, scale, 1.0f + 6.0f * scale, column, row);});
			enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, noBoundaryRange, 3, &boundary_event[1], &iter_event[1], ^{lin_solve(prevVVelocity, vVelocity, scale, 1.0f + 6.0f * scale, column, row);});
			enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, noBoundaryRange, 3, &boundary_event[2], &iter_event[2], ^{lin_solve(prevWVelocity, wVelocity, scale, 1.0f + 6.0f * scale, column, row);});
			boundary_event[0] = set_boundary(1, prevUVelocity, column, row, depth, 1, &iter_event[0]);
			boundary_event[1] = set_boundary(2, prevVVelocity, column, row, depth, 1, &iter_event[1]);
			boundary_event[2] = set_boundary(3, prevWVelocity, column, row, depth, 1, &iter_event[2]);
		}

		clk_event_t* project_event = project(prevUVelocity, prevVVelocity, prevWVelocity, uVelocity, vVelocity, N, column, row, depth, boundary_event);

		enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, noBoundaryRange, 3, project_event, &iter_event[0],
			^{advect(uVelocity, prevUVelocity, prevUVelocity, prevVVelocity, prevWVelocity, timeStep, N, column, row, depth);});
		boundary_event[0] = set_boundary(1, uVelocity, column, row, depth, 1, &iter_event[0]);

		enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, noBoundaryRange, 3, project_event, &iter_event[1],
			^{advect(vVelocity, prevVVelocity, prevUVelocity, prevVVelocity, prevWVelocity, timeStep, N, column, row, depth);});
		boundary_event[1] = set_boundary(2, vVelocity, column, row, depth, 1, &iter_event[1]);

		enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, noBoundaryRange, 3, project_event, &iter_event[2],
			^{advect(wVelocity, prevWVelocity, prevUVelocity, prevVVelocity, prevWVelocity, timeStep, N, column, row, depth);});
		boundary_event[2] = set_boundary(3, wVelocity, column, row, depth, 1, &iter_event[2]);

		project(uVelocity, vVelocity, wVelocity, prevUVelocity, prevVVelocity, N, column, row, depth, boundary_event);
	}
	);
} // ############################################################### end of OpenCL C code #####################################################################