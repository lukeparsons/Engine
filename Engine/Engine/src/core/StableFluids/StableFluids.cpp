#include "StableFluids.h"

#define MAX_ITERATIONS 20

void StableFluids::Simulate(float timeStep)
{
	// velocity
	addForces();
	advect();
	diffuse();
	project

	// density
}

void StableFluids::set_boundary(GridStructureHalo<float>& grid)
{
	for (unsigned int i = 0; i < column; i++)
	{
		for (unsigned int j = 0; j < row; j++)
		{
			for (unsigned int k = 0; k < depth; k++)
			{

			}
		}
	}
}

static unsigned int clamp(float val, unsigned int max)
{
	if(val < 0.5)
	{
		val = 0.5;
	}
	if(val > max + 0.5)
	{
		val = max + 0.5;
	}
	return static_cast<unsigned int>(val);
}

void StableFluids::advect(float timeStep, GridStructureHalo<float>& grid, GridStructureHalo<float>& prevGrid)
{
	float x, y, z, s0, t0, s1, t1, u0, u1;
	float dt0 = timeStep * N;

	unsigned int i0, j0, k0;
	for(unsigned int i = 0; i < column; i++)
	{
		for(unsigned int j = 0; j < row; j++)
		{
			for(unsigned int k = 0; k < depth; k++)
			{
				x = i - dt0 * uVelocity(i, j, k);
				y = j - dt0 * vVelocity(i, j, k);
				z = k - dt0 * wVelocity(i, j, k);

				i0 = clamp(x, column);
				j0 = clamp(y, row);
				k0 = clamp(z, depth);

				s1 = x - i0;
				s0 = 1.0f - s1;

				t1 = y - j0;
				t0 = 1.0f - t1;

				u1 = z - k0;
				u0 = 1.0f - u1;

				grid(i, j, k) =
					s0 *
					(t0 * (u0 * prevGrid(i0, j0, k0) + u1 * prevGrid(i0, j0, k0 + 1))
						+ (t1 * (u0 * prevGrid(i0, j0 + 1, k0) + u1 * prevGrid(i0, j0 + 1, k0 + 1))))
					+ s1 *
						(t0 * (u0 * prevGrid(i0 + 1, j0, k0) + u1 * prevGrid(i0 + 1, j0, k0 + 1)))
						+ (t1 * (u0 * prevGrid(i0 + 1, j0 + 1, k0)) + (u1 * prevGrid(i0 + 1, j0 + 1, k0 + 1)));

			}
		}
	}

	set_boundary();
}

void StableFluids::diffuse(float timeStep, float diff, GridStructureHalo<float>& grid, GridStructureHalo<float>& prevGrid)
{
	float scale = timeStep * diff * N;
	float c = 1.0f / (1 + 6 * scale);
	for(unsigned int t = 0; t < MAX_ITERATIONS; t++)
	{
		for(unsigned int i = 0; i < column; i++)
		{
			for(unsigned int j = 0; j < row; j++)
			{
				for(unsigned int k = 0; k < depth; k++)
				{
					grid(i, j, k) = prevGrid(i, j, k) + scale *
						(grid(i + 1, j, k) + grid(i - 1, j, k)
						+ grid(i, j + 1, k) + grid(i, j - 1, k)
						+ grid(i, j, k + 1) + grid(i, j, k - 1)) * c;
				}
			}
		}
	}

	set_boundary(grid);
}

void StableFluids::project(float timeStep, GridStructureHalo<float>& grid)
{
	float h = 1.0f / N;
	for(unsigned int i = 0; i < column; i++)
	{
		for(unsigned int j = 0; j < row; j++)
		{
			for(unsigned int k = 0; k < depth; k++)
			{
				grid(i, j, k) = -0.5f * h * (uVelocity(i + 1, j, k) - uVelocity(i - 1, j, k)
					+ vVelocity(i, j + 1, k) - vVelocity(i, j - 1, k)
					+ wVelocity(i, j, k + 1) - wVelocity(i, j, k - 1));
				pressure(i, j, k) = 0;
			}
		}
	}

	set_boundary(grid);
	set_boundary(pressure);

	diffuse(pressure);

	for(unsigned int i = 0; i < column; i++)
	{
		for(unsigned int j = 0; j < row; j++)
		{
			for(unsigned int k = 0; k < depth; k++)
			{
				uVelocity(i, j, k) -= 0.5 * (pressure(i + 1, j, k) - pressure(i - 1, j, k)) * N;
				vVelocity(i, j, k) -= 0.5 * (pressure(i, j + 1, k) - pressure(i, j - 1, k)) * N;
				wVelocity(i, j, k) -= 0.5 * (pressure(i, j, k + 1) - pressure(i, j, k - 1)) * N;
			}
		}
	}

	set_boundary(uVelocity);
	set_boundary(vVelocity);
	set_boundary(wVelocity);
}