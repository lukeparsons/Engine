#include "../../../opencl/kernel.hpp"

string main_code()
{
	return R( // ########################## begin of OpenCL C code ####################################################################

	kernel void add_source(global float* grid, global float* prevGrid, const float timeStep, const uint column, const uint row)
	{
		const uint i = get_global_id(0);
		const uint j = get_global_id(1);
		const uint k = get_global_id(2);
		uint idx = IX(i, j, k, column, row);

		grid[idx] += timeStep * prevGrid[idx];
	}

	kernel void add_velocity_sources(global float* uVelocity, global float* vVelocity, global float* wVelocity, global float* prevUVelocity, global float* prevVVelocity, global float* prevWVelocity,
		const float timeStep, const uint column, const uint row)
	{
		queue_t queue = get_default_queue();

		const uint i = get_global_id(0);
		const uint j = get_global_id(1);
		const uint k = get_global_id(2);
		uint idx = IX(i, j, k, column, row);

		uVelocity[idx] += timeStep * prevUVelocity[idx];
		vVelocity[idx] += timeStep * prevVVelocity[idx];
		wVelocity[idx] += timeStep * prevWVelocity[idx];
	}

	kernel void set_boundary(global float* grid, const int f1, const int f2, const int f3, const uint column, const uint row, const uint depth)
	{
		queue_t queue = get_default_queue();

		const size_t ones[2] = { 1, 1 };
		const size_t rd[2] = { row, depth };
		const size_t cd[2] = { column, depth };
		const size_t cr[2] = { column, row };

		ndrange_t sidesRange = ndrange_2D(ones, rd, ones);
		ndrange_t topBottomRange = ndrange_2D(ones, cd, ones);
		ndrange_t frontBackRange = ndrange_2D(ones, cr, ones);

		enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, frontBackRange, ^ {frontBackBoundaryFace(grid, f3, column, row, depth);});

		enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, topBottomRange, ^ { topBottomBoundaryFace(grid, f2, column, row); });

		enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, sidesRange, ^ { sidesBoundaryFace(grid, f1, column, row); });

		enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, ndrange_1D(1, column, 1), ^ { boundaryIEdge(grid, column, row, depth); });
		enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, ndrange_1D(1, row, 1), ^ { boundaryJEdge(grid, column, row, depth); });
		enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, ndrange_1D(1, depth, 1), ^ { boundaryKEdge(grid, column, row); });

		enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, ndrange_1D(1, 1, 1), ^ { boundaryCorners(grid, column, row, depth); });
	}

	/*clk_event_t* project(global float* uVelocity, global float* vVelocity, global float* wVelocity, global float* p, global float* div, const uint N,
		const uint column, const uint row, const uint depth, const uint wait_count, clk_event_t* wait_event)
	{
		queue_t queue = get_default_queue();

		const size_t ones[3] = { 1, 1, 1 };
		const size_t gridrange[3] = { column + 2, row + 2, depth + 2 };
		ndrange_t noBoundaryRange = ndrange_3D(ones, gridrange, ones);

		clk_event_t project1Event;
		enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, noBoundaryRange, wait_count, wait_event, &project1Event,
			^ { project1(uVelocity, vVelocity, wVelocity, p, div, N, column, row); });

		clk_event_t* pBoundary = set_boundary(div, column, row, depth, 1, &project1Event, 1, 1, 1);
		clk_event_t* divBoundary = set_boundary(p, column, row, depth, 1, &project1Event, 1, 1, 1);

		release_event(project1Event);

		clk_event_t boundary_events[2];
		enqueue_marker(queue, 7, pBoundary, &boundary_events[0]);
		enqueue_marker(queue, 7, divBoundary, &boundary_events[1]);

		release_events(pBoundary, 7);
		release_events(divBoundary, 7);

		clk_event_t iter_event;

		enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, noBoundaryRange, 2, boundary_events, &iter_event, ^ { lin_solve(p, div, 1.0f, 6.0f, column, row); });
		release_events(boundary_events, 2);
		clk_event_t* iter_boundary_events = set_boundary(p, column, row, depth, 1, &iter_event, 1, 1, 1);
		release_event(iter_event);

		for(int i = 0; i < MAX_ITERATIONS() - 1; i++)
		{
			enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, noBoundaryRange, 7, iter_boundary_events, &iter_event, ^ { lin_solve(p, div, 1.0f, 6.0f, column, row); });
			release_events(iter_boundary_events, 7);
			iter_boundary_events = set_boundary(p, column, row, depth, 1, &iter_event, 1, 1, 1);
			release_event(iter_event);
		}

		clk_event_t project2Event;
		enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, noBoundaryRange, 7, iter_boundary_events, &project2Event, ^ { project2(uVelocity, vVelocity, wVelocity, p, N, column, row); });
		release_events(iter_boundary_events, 7);

		clk_event_t* uEvents = set_boundary(uVelocity, column, row, depth, 1, &project2Event, -1, 1, 1);
		clk_event_t* vEvents = set_boundary(vVelocity, column, row, depth, 1, &project2Event, 1, -1, 1);
		clk_event_t* wEvents = set_boundary(wVelocity, column, row, depth, 1, &project2Event, 1, 1, -1);

		release_event(project2Event);

		clk_event_t velBoundaryEvents[3];
		enqueue_marker(queue, 3, uEvents, &velBoundaryEvents[0]);
		enqueue_marker(queue, 3, vEvents, &velBoundaryEvents[1]);
		enqueue_marker(queue, 3, wEvents, &velBoundaryEvents[2]);

		release_events(uEvents, 7);
		release_events(vEvents, 7);
		release_events(wEvents, 7);

		return velBoundaryEvents;
	}

	kernel void density_step(const float scale, global float* smoke, global float* prevSmoke, global float* uVelocity, global float* vVelocity, global float* wVelocity, const float timeStep,
		const uint N, const uint column, const uint row, const uint depth)
	{
		queue_t queue = get_default_queue();

		const size_t ones[3] = { 1, 1, 1 };
		const size_t gridrange[3] = { column + 2, row + 2, depth + 2 };
		ndrange_t addSourceRange = ndrange_3D(gridrange, ones);

		ndrange_t noBoundaryRange = ndrange_3D(ones, gridrange, ones);

		clk_event_t source_event;
		enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, addSourceRange, 0, NULL, &source_event, ^ { add_source(smoke, prevSmoke, timeStep, column, row); });

		clk_event_t* iter_event;
		enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, noBoundaryRange, 1, &source_event, iter_event,
			^ { lin_solve(prevSmoke, smoke, scale, 1.0f + 6.0f * scale, column, row); });

		release_event(source_event);
		
		clk_event_t* boundary_events = set_boundary(prevSmoke, column, row, depth, 1, iter_event, 1, 1, 1);
		release_event(*(iter_event));

		for(int i = 0; i < MAX_ITERATIONS() - 1; i++)
		{
			enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, noBoundaryRange, 7, boundary_events, iter_event, 
				^{lin_solve(prevSmoke, smoke, scale, 1.0f + 6.0f * scale, column, row);});
			release_events(boundary_events, 7);
			boundary_events = set_boundary(prevSmoke, column, row, depth, 1, iter_event, 1, 1, 1);
			release_event(*(iter_event));
		}
		
		clk_event_t advect_event;

		enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, noBoundaryRange, 7, boundary_events, &advect_event, ^ { advect(smoke, prevSmoke, uVelocity, vVelocity, wVelocity, timeStep, N, column, row, depth); });

		release_events(boundary_events, 7);

		boundary_events = set_boundary(smoke, column, row, depth, 1, &advect_event, 1, 1, 1); 
		release_events(boundary_events, 7);
		release_event(advect_event); 
	}

	kernel void velocity_step(const float scale, global float* uVelocity, global float* vVelocity, global float* wVelocity,
		global float* prevUVelocity, global float* prevVVelocity, global float* prevWVelocity,
		const float timeStep, const uint N, const uint column, const uint row, const uint depth)
	{
		queue_t queue = get_default_queue();

		const size_t ones[3] = { 1, 1, 1 };
		const size_t gridrange[3] = { column + 2, row + 2, depth + 2 };
		ndrange_t addSourceRange = ndrange_3D(gridrange, ones);

		// ADDSOURCE
		clk_event_t source_events[3];

		enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, addSourceRange, 0, NULL, &source_events[0], ^ { add_source(uVelocity, prevUVelocity, timeStep, column, row); });
		enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, addSourceRange, 0, NULL, &source_events[1], ^ { add_source(vVelocity, prevVVelocity, timeStep, column, row); });
		enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, addSourceRange, 0, NULL, &source_events[2], ^ { add_source(wVelocity, prevWVelocity, timeStep, column, row); });

		ndrange_t noBoundaryRange = ndrange_3D(ones, gridrange, ones);

		// LINSOLVE

		clk_event_t iter_events[3];
		enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, noBoundaryRange, 3, &source_events[0], &iter_events[0], ^ {lin_solve(prevUVelocity, uVelocity, scale, 1.0f + 6.0f * scale, column, row);});
		enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, noBoundaryRange, 3, &source_events[1], &iter_events[1], ^ {lin_solve(prevVVelocity, vVelocity, scale, 1.0f + 6.0f * scale, column, row);});
		enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, noBoundaryRange, 3, &source_events[2], &iter_events[2], ^ {lin_solve(prevWVelocity, wVelocity, scale, 1.0f + 6.0f * scale, column, row);});

		release_events(source_events, 3);

		clk_event_t* uBoundary_events = set_boundary(prevUVelocity, column, row, depth, 1, &iter_events[0], -1, 1, 1);
		clk_event_t* vBoundary_events = set_boundary(prevVVelocity, column, row, depth, 1, &iter_events[1], 1, -1, 1);
		clk_event_t* wBoundary_events = set_boundary(prevWVelocity, column, row, depth, 1, &iter_events[2], 1, 1, -1);

		release_events(iter_events, 3);

		for(int i = 0; i < MAX_ITERATIONS() - 1; i++)
		{
			enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, noBoundaryRange, 7, uBoundary_events, &iter_events[0], ^ { lin_solve(prevUVelocity, uVelocity, scale, 1.0f + 6.0f * scale, column, row); });
			enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, noBoundaryRange, 7, vBoundary_events, &iter_events[1], ^ { lin_solve(prevVVelocity, vVelocity, scale, 1.0f + 6.0f * scale, column, row); });
			enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, noBoundaryRange, 7, wBoundary_events, &iter_events[2], ^ { lin_solve(prevWVelocity, wVelocity, scale, 1.0f + 6.0f * scale, column, row); });

			release_events(uBoundary_events, 7);
			release_events(vBoundary_events, 7);
			release_events(wBoundary_events, 7);

			uBoundary_events = set_boundary(prevUVelocity, column, row, depth, 1, &iter_events[0], -1, 1, 1);
			vBoundary_events = set_boundary(prevVVelocity, column, row, depth, 1, &iter_events[1], 1, -1, 1);
			wBoundary_events = set_boundary(prevWVelocity, column, row, depth, 1, &iter_events[2], 1, 1, -1);

			release_events(iter_events, 3);
		}

		clk_event_t boundary_events[3];
		enqueue_marker(queue, 7, uBoundary_events, &boundary_events[0]);
		enqueue_marker(queue, 7, vBoundary_events, &boundary_events[1]);
		enqueue_marker(queue, 7, wBoundary_events, &boundary_events[2]);

		release_events(uBoundary_events, 7);
		release_events(vBoundary_events, 7);
		release_events(wBoundary_events, 7);

		// PROJECT

		//clk_event_t* project_events = project(prevUVelocity, prevVVelocity, prevWVelocity, uVelocity, vVelocity, N, column, row, depth, 3, boundary_events);

		release_events(boundary_events, 3);

		// ADVECT

		clk_event_t advect_events[3];

		enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, noBoundaryRange, 3, project_events, &advect_events[0],
			^ { advect(uVelocity, prevUVelocity, prevUVelocity, prevVVelocity, prevWVelocity, timeStep, N, column, row, depth); });
		uBoundary_events = set_boundary(uVelocity, column, row, depth, 1, &advect_events[0], -1, 1, 1);

		enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, noBoundaryRange, 3, project_events, &advect_events[1],
			^ { advect(vVelocity, prevVVelocity, prevUVelocity, prevVVelocity, prevWVelocity, timeStep, N, column, row, depth); });
		vBoundary_events = set_boundary(vVelocity, column, row, depth, 1, &advect_events[1], 1, -1, 1);

		int a = 0;
		enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, noBoundaryRange, 3, project_events, &advect_events[2],
			^ { advect(wVelocity, prevWVelocity, prevUVelocity, prevVVelocity, prevWVelocity, timeStep, N, column, row, depth); });
		//printf("%d", a);
		wBoundary_events = set_boundary(wVelocity, column, row, depth, 1, &advect_events[2], 1, 1, -1);

		release_events(project_events, 3);
		release_events(advect_events, 3);

		enqueue_marker(queue, 7, uBoundary_events, &boundary_events[0]);
		enqueue_marker(queue, 7, vBoundary_events, &boundary_events[1]);
		enqueue_marker(queue, 7, wBoundary_events, &boundary_events[2]);

		// PROJECT

		project_events = project(uVelocity, vVelocity, wVelocity, prevUVelocity, prevVVelocity, N, column, row, depth, 3, boundary_events);

		release_events(boundary_events, 3);
		release_events(uBoundary_events, 7);
		release_events(vBoundary_events, 7);
		release_events(wBoundary_events, 7);
		release_events(project_events, 3);

	} */
	);
}