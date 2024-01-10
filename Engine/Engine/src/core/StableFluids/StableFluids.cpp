#include "StableFluids.h"
#include "GridStructures.h"

#define MAX_ITERATIONS 10

#define SWAP(x0, x) {GridStructure<float>* tmp = x0; x0 = x; x = tmp;}

inline static void SwapPointer(GridStructure<float>** a, GridStructure<float>** b)
{
	GridStructure<float>* c = *a;
	*a = *b;
	*b = c;
}

void StableFluids::Simulate(float timeStep, float diffRate)
{
	/*diffuse(timeStep, diffRate, uVelocity, prevUVelocity, 1);
	diffuse(timeStep, diffRate, vVelocity, prevVVelocity, 2);
	diffuse(timeStep, diffRate, vVelocity, prevVVelocity, 3);

	project(timeStep, prevUVelocity, prevVVelocity, prevWVelocity, uVelocity, vVelocity);

	advect(timeStep, uVelocity, prevUVelocity, uVelocity, vVelocity, wVelocity, 1);
	advect(timeStep, vVelocity, prevVVelocity, uVelocity, vVelocity, wVelocity, 2);
	advect(timeStep, wVelocity, prevWVelocity, uVelocity, vVelocity, wVelocity, 3);

	project(timeStep, uVelocity, vVelocity, wVelocity, prevUVelocity, prevVVelocity);

	diffuse(timeStep, diffRate, prevSmoke, smoke, 0);
	advect(timeStep, smoke, prevSmoke, uVelocity, vVelocity, wVelocity, 0); */

	for(unsigned int i = 1; i < column - 1; i++)
	{
		for(unsigned int j = 1; j < row - 1; j++)
		{
			for(unsigned int k = 1; k < depth - 1; k++)
			{
				//std::cout << smoke(i, j, k) << ", " << smoke(i, j, k) << std::endl;
			}
		}
	}

	velocity_step(timeStep, &uVelocity, &vVelocity, &wVelocity, &prevUVelocity, &prevVVelocity, &prevWVelocity);
	density_step(timeStep, diffRate, &smoke, &prevSmoke);
	for(unsigned int i = 0; i < column; i++)
	{
		for(unsigned int j = 0; j < row; j++)
		{
			for(unsigned int k = 0; k < depth; k++)
			{
				uVelocity(i, j, k) += 100.f;
				std::cout << uVelocity(i, j, k) << std::endl;
			}
		}
	}
}

void StableFluids::add_source(float timeStep, GridStructure<float>& grid, GridStructure<float>& prevGrid)
{
	for(unsigned int i = 0; i < column; i++)
	{
		for(unsigned int j = 0; j < row; j++)
		{
			for(unsigned int k = 0; k < depth; k++)
			{
				grid(i, j, k) += timeStep * prevGrid(i, j, k);
			}
		}
	}
}

void StableFluids::density_step(float timeStep, float diffRate, GridStructure<float>* density, GridStructure<float>* prevDensity)
{
	add_source(timeStep, *density, *prevDensity);
	SWAP(density, prevDensity);
	diffuse(timeStep, diffRate, *density, *prevDensity, 0);
	SWAP(density, prevDensity);
	advect(timeStep, *density, *prevDensity, uVelocity, vVelocity, wVelocity, 0);
}

void StableFluids::velocity_step(float timeStep, GridStructure<float>* uVel, GridStructure<float>* vVel, GridStructure<float>* wVel, 
	GridStructure<float>* prevUVel, GridStructure<float>* prevVVel, GridStructure<float>* prevWVel)
{
	add_source(timeStep, *uVel, *prevUVel);
	add_source(timeStep, *vVel, *prevVVel);
	add_source(timeStep, *wVel, *prevWVel);

	SWAP(uVel, prevUVel);
	diffuse(timeStep, viscosity, *uVel, *prevUVel, 1);
	SWAP(vVel, prevVVel);
	diffuse(timeStep, viscosity, *vVel, *prevVVel, 2);
	SWAP(wVel, prevWVel);
	diffuse(timeStep, viscosity, *wVel, *prevWVel, 3);
	

	project(timeStep, *uVel, *vVel, *wVel, *prevUVel, *prevVVel);

	SWAP(uVel, prevUVel);
	SWAP(vVel, prevVVel);
	SWAP(wVel, prevWVel);
	advect(timeStep, *uVel, *prevUVel, *prevUVel, *prevVVel, *prevWVel, 1);
	advect(timeStep, *vVel, *prevVVel, *prevUVel, *prevVVel, *prevWVel, 2);
	advect(timeStep, *wVel, *prevWVel, *prevUVel, *prevVVel, *prevWVel, 3);
	project(timeStep, *uVel, *vVel, *wVel, *prevUVel, *prevVVel);
}

void StableFluids::set_boundary(GridStructure<float>& grid, unsigned int b)
{
	for(unsigned int i = 1; i < column - 1; i++)
	{
		for(unsigned int j = 1; j < row - 1; j++)
		{
			// Front and back walls
			grid(i, j, 0) = b == 3 ? -grid(i, j, 1) : grid(i, j, 1);
			grid(i, j, depth - 1) = b == 3 ? -grid(i, j, depth - 2) : grid(i, j, depth - 2);
		}
	}

	for(unsigned int i = 1; i < column - 1; i++)
	{
		for(unsigned int k = 1; k < depth - 1; k++)
		{
			// Top and bottom walls
			grid(i, 0, k) = b == 2 ? -grid(i, 1, k) : grid(i, 1, k);
			grid(i, row - 1, k) = b == 2 ?  -grid(i, row - 2, k) : grid(i, row - 2, k);
		}
	}

	for(unsigned int j = 1; j < row - 1; j++)
	{
		for(unsigned int k = 1; k < depth - 1; k++)
		{
			// Left and right walls
			grid(0, j, k) = b == 1 ?  -grid(1, j, k) : grid(1, j, k);
			grid(column - 1, j, k) = b == 1 ? -grid(column - 2, j, k) : grid(column - 2, j, k);
		}
	}

	for(unsigned int i = 1; i < depth - 1; i++)
	{
		grid(i, 0, 0) = 0.5f * (grid(i, 1, 0) + grid(i, 0, 1));
		grid(i, row - 1, 0) = 0.5f * (grid(i, row - 2, 0) + grid(i, row - 1, 1));
		grid(i, 0, depth - 1) = 0.5f * (grid(i, 0, depth - 2) + grid(i, 1, depth - 1));
		grid(i, row - 1, depth - 1) = 0.5f * (grid(i, row - 2, depth - 1) + grid(i, row - 1, depth - 2));
	}

	for(unsigned int j = 1; j < row - 1; j++)
	{
		grid(0, j, 0) = 0.5f * (grid(1, j, 0) + grid(0, j, 1));
		grid(column - 1, j, 0) = 0.5f * (grid(column - 2, j, 0) + grid(column - 1, j, 1));
		grid(0, j, depth - 1) = 0.5f * (grid(0, j, depth - 2) + grid(1, j, depth - 1));
		grid(column - 1, j, depth - 1) = 0.5f * (grid(column - 2, j, depth - 1) + grid(column - 1, j, depth - 2));
	}

	for(unsigned int k = 1; k < depth - 1; k++)
	{
		grid(0, 0, k) = 0.5f * (grid(0, 1, k) + grid(1, 0, k));
		grid(0, row - 1, k) = 0.5f * (grid(0, row - 2, k) + grid(1, row - 1, k));
		grid(column - 1, 0, k) = 0.5f * (grid(column - 2, 0, k) + grid(column - 1, 1, k));
		grid(column - 1, row - 1, k) = 0.5f * (grid(column - 1, row - 2, k) + grid(column - 2, row - 1, k));
	}

	// Corners
	grid(0, 0, 0) = 0.33f * (grid(1, 0, 0) + grid(0, 1, 0) + grid(0, 0, 1));
	grid(0, row - 1, 0) = 0.33f * (grid(1, row - 1, 0) + grid(0, row - 2, 0) + grid(0, row - 1, 1));
	grid(0, 0, depth - 1) = 0.33f * (grid(1, 0, depth - 1) + grid(0, 1, depth - 1) + grid(0, 0, depth - 2));
	grid(0, row - 1, depth - 1) = 0.33f * (grid(1, row - 1, depth - 1) + grid(0, row - 2, depth - 1) + grid(0, row - 1, depth - 2));
	grid(column - 1, 0, 0) = 0.33f * (grid(column - 2, 0, 0) + grid(column - 1, 1, 0) + grid(column - 1, 0, 1));
	grid(column - 1, row - 1, 0) = 0.33f * (grid(column - 2, row - 2, 0) + grid(column - 1, row - 2, 0) + grid(column - 1, row - 1, 1));
	grid(column - 1, 0, depth - 1) = 0.33f * (grid(column - 2, 0, depth - 1) + grid(column - 1, 1, depth - 1) + grid(column - 1, 0, depth - 2));
	grid(column - 1, row - 1, depth - 1) = 0.33f * (grid(column - 2, row - 1, depth - 1) + grid(column - 1, row - 2, depth - 1) + grid(column - 1, row - 1, depth - 2));

}

static unsigned int clamp(float val, unsigned int max)
{
	if(val < 0.5f)
	{
		val = 0.5f;
	}
	if(val > max + 0.5f)
	{
		val = max + 0.5f;
	}
	return static_cast<unsigned int>(val);
}

void StableFluids::advect(float timeStep, GridStructure<float>& grid, GridStructure<float>& prevGrid, GridStructure<float>& uVel, GridStructure<float>& vVel, GridStructure<float>& wVel, unsigned int b)
{
	float x, y, z, s0, t0, s1, t1, u0, u1;
	float dt0 = timeStep * N;

	unsigned int i0, j0, k0;
	for(unsigned int i = 1; i < column - 1; i++)
	{
		for(unsigned int j = 1; j < row - 1; j++)
		{
			for(unsigned int k = 1; k < depth - 1; k++)
			{
				x = i - dt0 * uVel(i, j, k);
				y = j - dt0 * vVel(i, j, k);
				z = k - dt0 * wVel(i, j, k);

				i0 = clamp(x, column - 2);
				j0 = clamp(y, row - 2);
				k0 = clamp(z, depth - 2);

				s1 = x - i0;
				s0 = 1.0f - s1;

				t1 = y - j0;
				t0 = 1.0f - t1;

				u1 = z - k0;
				u0 = 1.0f - u1;

				grid(i, j, k) = s0 * (t0 * u0 * prevGrid(i0, j0, k0) + t1 * u0 * prevGrid(i0, j0 + 1, k) + t0 * u1 * prevGrid(i0, j0, k0 + 1) + t1 * u1 * prevGrid(i0, j0 + 1, k0 + 1))
					+ s1 * (t0 * u0 * prevGrid(i0 + 1, j0, k0) + t1 * u0 * prevGrid(i0 + 1, j0 + 1, k0) + t0 * u1 * prevGrid(i0 + 1, j0, k0 + 1) + t1 * u1 * prevGrid(i0 + 1, j0 + 1, k0 + 1));

			}
		}
	}

	set_boundary(grid, b);
}

void StableFluids::diffuse(float timeStep, float diffRate, GridStructure<float>& grid, GridStructure<float>& prevGrid, unsigned int b)
{
	float scale = timeStep * diffRate * N * N * N;
	lin_solve(scale, grid, prevGrid, b, 1 + 6 * scale);
}

void StableFluids::lin_solve(float scale, GridStructure<float>& grid, GridStructure<float>& prevGrid, unsigned int b, float c)
{
	c = 1.0f / c;
	for(unsigned int t = 0; t < MAX_ITERATIONS; t++)
	{
		for(unsigned int i = 1; i < column - 1; i++)
		{
			for(unsigned int j = 1; j < row - 1; j++)
			{
				for(unsigned int k = 1; k < depth - 1; k++)
				{
					grid(i, j, k) = prevGrid(i, j, k) + scale *
						(grid(i + 1, j, k) + grid(i - 1, j, k)
							+ grid(i, j + 1, k) + grid(i, j - 1, k)
							+ grid(i, j, k + 1) + grid(i, j, k - 1)) * c;
				}
			}
		}
	}

	set_boundary(grid, b);
}

void StableFluids::project(float timeStep, GridStructure<float>& uVel, GridStructure<float>& vVel, GridStructure<float>& wVel, GridStructure<float>& div, GridStructure<float>& p)
{
	float h = 1.0f / N;
	for(unsigned int i = 1; i < column - 1; i++)
	{
		for(unsigned int j = 1; j < row - 1; j++)
		{
			for(unsigned int k = 1; k < depth - 1; k++)
			{
				div(i, j, k) = -0.33f * h * (uVel(i + 1, j, k) - uVel(i - 1, j, k)
					+ vVel(i, j + 1, k) - vVel(i, j - 1, k)
					+ wVel(i, j, k + 1) - wVel(i, j, k - 1));
				p(i, j, k) = 0;
			}
		}
	}

	set_boundary(div, 0);
	set_boundary(p, 0);

	lin_solve(1, p, div, 0, 6);

	for(unsigned int i = 1; i < column - 1; i++)
	{
		for(unsigned int j = 1; j < row - 1; j++)
		{
			for(unsigned int k = 1; k < depth - 1; k++)
			{
				uVel(i, j, k) -= 0.5f * (p(i + 1, j, k) - p(i - 1, j, k)) * N;
				vVel(i, j, k) -= 0.5f * (p(i, j + 1, k) - p(i, j - 1, k)) * N;
				wVel(i, j, k) -= 0.5f * (p(i, j, k + 1) - p(i, j, k - 1)) * N;
			}
		}
	}

	set_boundary(uVel, 1);
	set_boundary(vVel, 2);
	set_boundary(wVel, 3);
}