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

	kernel void advect(const int b, global float* grid, global float* prevGrid, global float* u, global float* v, global float* w, const float timeStep)
	{
		uint i, j, k, 
	}

	void StableFluids::advect(int b, GridStructure<float>& grid, GridStructure<float>& prevGrid, GridStructure<float>& u, GridStructure<float>& v, GridStructure<float>& w, float timeStep)
	{
		int i, j, k, i0, j0, k0, i1, j1, k1;
		float x, y, z, s0, t0, s1, t1, u1, u0, dtx, dty, dtz;

		dtx = dty = dtz = timeStep * N;

		for(i = 1; i <= column; i++)
		{
			for(j = 1; j <= row; j++)
			{
				for(k = 1; k <= depth; k++)
				{
					x = i - dtx * u(i, j, k);
					y = j - dty * v(i, j, k);
					z = k - dtz * w(i, j, k);
					if(x < 0.5f)
					{
						x = 0.5f;
					}
					if(x > column + 0.5f)
					{
						x = column + 0.5f;
					}
					i0 = (int)x;
					i1 = i0 + 1;
					if(y < 0.5f)
					{
						y = 0.5f;
					}
					if(y > row + 0.5f)
					{
						y = row + 0.5f;
					}
					j0 = (int)y;
					j1 = j0 + 1;
					if(z < 0.5f)
					{
						z = 0.5f;
					} if(z > depth + 0.5f)
					{
						z = depth + 0.5f;
					}
					k0 = (int)z;
					k1 = k0 + 1;

					s1 = x - i0;
					s0 = 1 - s1;
					t1 = y - j0;
					t0 = 1 - t1;
					u1 = z - k0;
					u0 = 1 - u1;
					grid(i, j, k) = s0 * (t0 * u0 * prevGrid(i0, j0, k0) + t1 * u0 * prevGrid(i0, j1, k0) + t0 * u1 * prevGrid(i0, j0, k1) + t1 * u1 * prevGrid(i0, j1, k1)) +
						s1 * (t0 * u0 * prevGrid(i1, j0, k0) + t1 * u0 * prevGrid(i1, j1, k0) + t0 * u1 * prevGrid(i1, j0, k1) + t1 * u1 * prevGrid(i1, j1, k1));
				}
			}
		}
		set_boundary(b, grid);
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