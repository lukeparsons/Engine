#include "../../../opencl/kernel.hpp"

string actions_code()
{
	return R( // ########################## begin of OpenCL C code ####################################################################

		#define IX(i,j,k,column,row)((i)+(column+2)*(j)+(column+2)*(row+2)*(k))

		kernel void advect(const float timeStep, global float* grid, global float* prevGrid, global float* u, global float* v, global float* w,
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
			}
			if(z > depth + 0.5f)
			{
				z = depth + 0.5f;
			}
			int k0 = (int)z;
			int k1 = k0 + 1;

			float s1 = x - i0;
			float s0 = 1 - s1;
			float t1 = y - j0;
			float t0 = 1 - t1;
			float u1 = z - k0;
			float u0 = 1 - u1;
			grid[idx] = s0 * (t0 * u0 * prevGrid[IX(i0, j0, k0, column, row)] + t1 * u0 * prevGrid[IX(i0, j1, k0, column, row)] + t0 * u1 * prevGrid[IX(i0, j0, k1, column, row)]
				+ t1 * u1 * prevGrid[IX(i0, j1, k1, column, row)]) + s1 * (t0 * u0 * prevGrid[IX(i1, j0, k0, column, row)]
					+ t1 * u0 * prevGrid[IX(i1, j1, k0, column, row)] + t0 * u1 * prevGrid[IX(i1, j0, k1, column, row)] + t1 * u1 * prevGrid[IX(i1, j1, k1, column, row)]);
	}

	kernel void lin_solve(global float* grid, global float* prevGrid, const float a, const float c, const uint column, const uint row)
	{
		const uint i = get_global_id(0);
		const uint j = get_global_id(1);
		const uint k = get_global_id(2);
		uint idx = IX(i, j, k, column, row);

		uint left = idx - 1;
		uint right = idx + 1;
		uint up = IX(i, j + 1, k, column, row);
		uint down = IX(i, j - 1, k, column, row);
		uint behind = IX(i, j, k + 1, column, row);
		uint infront = IX(i, j, k - 1, column, row);

		grid[idx] = (prevGrid[idx] + a *
			(grid[left] + grid[right]
				+ grid[down] + grid[up]
				+ grid[behind] + grid[infront])) / c;
	}

	kernel void project1(global float* u, global float* v, global float* w, global float* p, global float* div, const uint N, const uint column, const uint row)
	{
		const uint i = get_global_id(0);
		const uint j = get_global_id(1);
		const uint k = get_global_id(2);

		uint idx = IX(i, j, k, column, row);

		uint left = idx - 1;
		uint right = idx + 1;
		uint up = IX(i, j + 1, k, column, row);
		uint down = IX(i, j - 1, k, column, row);
		uint behind = IX(i, j, k + 1, column, row);
		uint infront = IX(i, j, k - 1, column, row);

		div[idx] = -(1.0f / 3.0f) * ((u[right] - u[left]) + (v[up] - v[down]) + (w[behind] - w[infront])) / N;
		p[idx] = 0.0f;
	}

	kernel void project2(global float* u, global float* v, global float* w, global float* p, const uint N, const uint column, const uint row)
	{
		const uint i = get_global_id(0);
		const uint j = get_global_id(1);
		const uint k = get_global_id(2);

		uint idx = IX(i, j, k, column, row);

		uint left = idx - 1;
		uint right = idx + 1;
		uint up = IX(i, j + 1, k, column, row);
		uint down = IX(i, j - 1, k, column, row);
		uint behind = IX(i, j, k + 1, column, row);
		uint infront = IX(i, j, k - 1, column, row);

		u[idx] -= 0.5f * N * (p[right] - p[left]);
		v[idx] -= 0.5f * N * (p[up] - p[down]);
		w[idx] -= 0.5f * N * (p[behind] - p[infront]);
	}

	kernel void sidesBoundaryFace(global float* grid, const int is1B, const uint column, const uint row)
	{
		const uint j = get_global_id(0);
		const uint k = get_global_id(1);

		grid[IX(0, j, k, column, row)] = grid[IX(1, j, k, column, row)] * is1B;
		grid[IX(column + 1, j, k, column, row)] = grid[IX(column, j, k, column, row)] * is1B;
	}

	kernel void topBottomBoundaryFace(global float* grid, const int is2B, const uint column, const uint row)
	{
		const uint i = get_global_id(0);
		const uint k = get_global_id(1);

		grid[IX(i, 0, k, column, row)] = grid[IX(i, 1, k, column, row)] * is2B;
		grid[IX(i, row + 1, k, column, row)] = grid[IX(i, row, k, column, row)] * is2B;
	}

	kernel void frontBackBoundaryFace(global float* grid, const int is3B, const uint column, const uint row, const uint depth)
	{
		const uint i = get_global_id(0);
		const uint j = get_global_id(1);

		grid[IX(i, j, 0, column, row)] = grid[IX(i, j, 1, column, row)] * is3B;
		grid[IX(i, j, depth + 1, column, row)] = grid[IX(i, j, depth, column, row)] * is3B;
	}

	kernel void boundaryIEdge(global float* grid, const uint column, const uint row, const uint depth)
	{
		const uint i = get_global_id(0);

		grid[IX(i, 0, 0, column, row)] = 0.5f * (grid[IX(i, 1, 0, column, row)] + grid[IX(i, 0, 1, column, row)]);
		grid[IX(i, row + 1, 0, column, row)] = 0.5f * (grid[IX(i, row, 0, column, row)] + grid[IX(i, row + 1, 1, column, row)]);
		grid[IX(i, 0, depth + 1, column, row)] = 0.5f * (grid[IX(i, 0, depth, column, row)] + grid[IX(i, 1, depth + 1, column, row)]);
		grid[IX(i, row + 1, depth + 1, column, row)] = 0.5f * (grid[IX(i, row, depth + 1, column, row)] + grid[IX(i, row + 1, depth, column, row)]);
	}

	kernel void boundaryJEdge(global float* grid, const uint column, const uint row, const uint depth)
	{
		const uint j = get_global_id(0);

		grid[IX(0, j, 0, column, row)] = 0.5f * (grid[IX(1, j, 0, column, row)] + grid[IX(0, j, 1, column, row)]);
		grid[IX(column + 1, j, 0, column, row)] = 0.5f * (grid[IX(column, j, 0, column, row)] + grid[IX(column + 1, j, 1, column, row)]);
		grid[IX(0, j, depth + 1, column, row)] = 0.5f * (grid[IX(0, j, depth, column, row)] + grid[IX(1, j, depth + 1, column, row)]);
		grid[IX(column + 1, j, depth + 1, column, row)] = 0.5f * (grid[IX(column, j, depth + 1, column, row)] + grid[IX(column + 1, j, depth, column, row)]);
	}

	kernel void boundaryKEdge(global float* grid, const uint column, const uint row)
	{
		const uint k = get_global_id(0);

		grid[IX(0, 0, k, column, row)] = 0.5f * (grid[IX(0, 1, k, column, row)] + grid[IX(1, 0, k, column, row)]);
		grid[IX(0, row + 1, k, column, row)] = 0.5f * (grid[IX(0, row, k, column, row)] + grid[IX(1, row + 1, k, column, row)]);
		grid[IX(column + 1, 0, k, column, row)] = 0.5f * (grid[IX(column, 0, k, column, row)] + grid[IX(column + 1, 1, k, column, row)]);
		grid[IX(column + 1, row + 1, k, column, row)] = 0.5f * (grid[IX(column + 1, row, k, column, row)] + grid[IX(column, row + 1, k, column, row)]);
	}

	kernel void boundaryCorners(global float* grid, const uint column, const uint row, const uint depth)
	{
		grid[IX(0, 0, 0, column, row)] = (1.0f / 3.0f) * (grid[IX(1, 0, 0, column, row)] + grid[IX(0, 1, 0, column, row)] + grid[IX(0, 0, 1, column, row)]);
		grid[IX(0, row + 1, 0, column, row)] = (1.0f / 3.0f) * (grid[IX(1, row + 1, 0, column, row)] + grid[IX(0, row, 0, column, row)] + grid[IX(0, row + 1, 1, column, row)]);

		grid[IX(column + 1, 0, 0, column, row)] = (1.0f / 3.0f) * (grid[IX(column, 0, 0, column, row)] + grid[IX(column + 1, 1, 0, column, row)] + grid[IX(column + 1, 0, 1, column, row)]);
		grid[IX(column + 1, row + 1, 0, column, row)] = (1.0f / 3.0f) * (grid[IX(column, row + 1, 0, column, row)] + grid[IX(column + 1, row, 0, column, row)] + grid[IX(column + 1, row + 1, 1, column, row)]);

		grid[IX(0, 0, depth + 1, column, row)] = (1.0f / 3.0f) * (grid[IX(1, 0, depth + 1, column, row)] + grid[IX(0, 1, depth + 1, column, row)] + grid[IX(0, 0, depth, column, row)]);
		grid[IX(0, row + 1, depth + 1, column, row)] = (1.0f / 3.0f) * (grid[IX(1, row + 1, depth + 1, column, row)] + grid[IX(0, row, depth + 1, column, row)] + grid[IX(0, row + 1, depth, column, row)]);

		grid[IX(column + 1, 0, depth + 1, column, row)] = (1.0f / 3.0f) * (grid[IX(column, 0, depth + 1, column, row)] + grid[IX(column + 1, 1, depth + 1, column, row)] + grid[IX(column + 1, 0, depth, column, row)]);
		grid[IX(column + 1, row + 1, depth + 1, column, row)] = (1.0f / 3.0f) * (grid[IX(column, row + 1, depth + 1, column, row)] + grid[IX(column + 1, row, depth + 1, column, row)] + grid[IX(column + 1, row + 1, depth, column, row)]);
	}
	);
}