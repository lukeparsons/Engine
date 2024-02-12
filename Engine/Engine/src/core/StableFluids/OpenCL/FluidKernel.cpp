#include "../../../opencl/kernel.hpp"

string opencl_c_container()
{
	return R( // ########################## begin of OpenCL C code ####################################################################

	#define IX(i,j,k,column,row)((i)+(j)*(column)+(k)*(column)*(row))

	void lin_solve(const int b, global float* grid, global float* prevGrid, const float a, const float c, const uint column, const uint row)
	{
		const uint i = get_global_id(0);
		const uint j = get_global_id(1);
		const uint k = get_global_id(2);
		uint idx = IX(i, j, k, column, row);

		uint left = idx - 1;
		uint right = idx + 1;
		uint up = idx + row;
		uint down = idx - row;
		uint behind = idx + column * row;
		uint infront = idx - column * row;

		const float invC = 1.0f / c;

		grid[idx] = (prevGrid[idx] + a *
			(grid[left] + grid[right]
			+ grid[down] + grid[up]
			+ grid[behind] + grid[infront])) * invC;
	} 

	kernel void diffuse(const int b, global float* grid, global float* prevGrid, const float timeStep, const float viscosity, const uint N, const uint column, const uint row)
	{
		const float scale = timeStep * viscosity * N * N * N;
		lin_solve(b, grid, prevGrid, scale, 1 + 6 * scale, column, row);
	}

	kernel void advect(const int b, global float* grid, global float* prevGrid, global float* u, global float* v, global float* w, const float timeStep,
		const uint N, const uint column, const uint row, const uint depth)
	{
		const uint i = get_global_id(0);
		const uint j = get_global_id(1);
		const uint k = get_global_id(2);
		uint idx = IX(i, j, k, column, row);

		const float dtx = timeStep * N;

		float x = i - dtx * u[idx];
		float y = j - dtx * v[idx];
		float z = k - dtx * w[idx];
		if(x < 0.5f)
		{
			x = 0.5f;
		}
		if(x > column + 0.5f)
		{
			x = column + 0.5f;
		}
		int i0 = (int)x;
		int i1 = i0 + 1;
		if(y < 0.5f)
		{
			y = 0.5f;
		}
		if(y > row + 0.5f)
		{
			y = row + 0.5f;
		}
		int j0 = (int)y;
		int j1 = j0 + 1;
		if(z < 0.5f)
		{
			z = 0.5f;
		} if(z > depth + 0.5f)
		{
			z = depth + 0.5f;
		}
		int k0 = (int)z;
		int k1 = k0 + 1;

		int s1 = x - i0;
		int s0 = 1 - s1;
		int t1 = y - j0;
		int t0 = 1 - t1;
		int u1 = z - k0;
		int u0 = 1 - u1;
		//grid[idx] = s0 * (t0 * u0 * prevGrid(i0, j0, k0) + t1 * u0 * prevGrid(i0, j1, k0) + t0 * u1 * prevGrid(i0, j0, k1) + t1 * u1 * prevGrid(i0, j1, k1)) +
			//s1 * (t0 * u0 * prevGrid(i1, j0, k0) + t1 * u0 * prevGrid(i1, j1, k0) + t0 * u1 * prevGrid(i1, j0, k1) + t1 * u1 * prevGrid(i1, j1, k1));

		// set_boundary after
	}



	kernel void add_source(global float* grid, global float* prevGrid, const float timeStep, const uint column, const uint row)
	{
		const uint i = get_global_id(0);
		const uint j = get_global_id(1);
		const uint k = get_global_id(2);
		uint idx = IX(i, j, k, column, row);

		uint left = idx - 1;
		uint right = idx + 1;
		uint up = idx + row;
		uint down = idx - row;
		uint behind = idx + column * row;
		uint infront = idx - column * row;

		grid[idx] = prevGrid[left];
	}

	);
} // ############################################################### end of OpenCL C code #####################################################################