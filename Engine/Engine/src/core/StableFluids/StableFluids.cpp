#include "StableFluids.h"

#define SWAP(x0, x) {GridStructure<float>* tmp = x0; x0 = x; x = tmp;}

#define MAX_ITERATIONS 10

void StableFluids::add_source(GridStructure<float>& grid, GridStructure<float>& prevGrid, float timeStep)
{
	

	for(int i = 0; i <= column + 1; i++)
	{
		for(int j = 0; j <= row + 1; j++)
		{
			
			for(int k = 0; k <= depth + 1; k++)
			{
				grid(i, j, k) += timeStep * prevGrid(i, j, k);
			}
		}
	}
}

void StableFluids::set_boundary(int b, GridStructure<float>& grid)
{
	int i, j, k;

	for(i = 1; i <= column; i++)
	{
		for(j = 1; j <= row; j++)
		{
			grid(i, j, 0) = b == 3 ? -grid(i, j, 1) : grid(i, j, 1);
			grid(i, j, depth + 1) = b == 3 ? -grid(i, j, depth) : grid(i, j, depth);
		}
	}

	
	for(j = 1; j <= row; j++)
	{
		
		for(k = 1; k <= depth; k++)
		{
			grid(0, j, k) = b == 1 ? -grid(1, j, k) : grid(1, j, k);
			grid(column + 1, j, k) = b == 1 ? -grid(column, j, k) : grid(column, j, k);
		}
	}

	
	for(i = 1; i <= column; i++)
	{
		
		for(k = 1; k <= depth; k++)
		{
			grid(i, 0, k) = b == 2 ? -grid(i, 1, k) : grid(i, 1, k);
			grid(i, row + 1, k) = b == 2 ? -grid(i, row, k) : grid(i, row, k);
		}
	}

	for(i = 1; i <= column; i++)
	{
		grid(i, 0, 0) = 0.5f * (grid(i, 1, 0) + grid(i, 0, 1));
		grid(i, row + 1, 0) = 0.5f * (grid(i, row, 0) + grid(i, row + 1, 1));
		grid(i, 0, depth + 1) = 0.5f * (grid(i, 0, depth) + grid(i, 1, depth + 1));
		grid(i, row + 1, depth + 1) = 0.5f * (grid(i, row, depth + 1) + grid(i, row + 1, depth));
	}
	
	for(j = 1; j <= row; j++)
	{
		grid(0, j, 0) = 0.5f * (grid(1, j, 0) + grid(0, j, 1));
		grid(column + 1, j, 0) = 0.5f * (grid(column, j, 0) + grid(column + 1, j, 1));
		grid(0, j, depth + 1) = 0.5f * (grid(0, j, depth) + grid(1, j, depth + 1));
		grid(column + 1, j, depth + 1) = 0.5f * (grid(column, j, depth + 1) + grid(column + 1, j, depth));
	}

	for(k = 1; k <= depth; k++)
	{
		grid(0, 0, k) = 0.5f * (grid(0, 1, k) + grid(1, 0, k));
		grid(0, row + 1, k) = 0.5f * (grid(0, row, k) + grid(1, row + 1, k));
		grid(column + 1, 0, k) = 0.5f * (grid(column, 0, k) + grid(column + 1, 1, k));
		grid(column + 1, row + 1, k) = 0.5f * (grid(column + 1, row, k) + grid(column, row + 1, k));
	}

	grid(0, 0, 0) = (1.0f / 3.0f) * (grid(1, 0, 0) + grid(0, 1, 0) + grid(0, 0, 1));
	grid(0, row + 1, 0) = (1.0f / 3.0f) * (grid(1, row + 1, 0) + grid(0, row, 0) + grid(0, row + 1, 1));

	grid(column + 1, 0, 0) = (1.0f / 3.0f) * (grid(column, 0, 0) + grid(column + 1, 1, 0) + grid(column + 1, 0, 1));
	grid(column + 1, row + 1, 0) = (1.0f / 3.0f) * (grid(column, row + 1, 0) + grid(column + 1, row, 0) + grid(column + 1, row + 1, 1));

	grid(0, 0, depth + 1) = (1.0f / 3.0f) * (grid(1, 0, depth + 1) + grid(0, 1, depth + 1) + grid(0, 0, depth));
	grid(0, row + 1, depth + 1) = (1.0f / 3.0f) * (grid(1, row + 1, depth + 1) + grid(0, row, depth + 1) + grid(0, row + 1, depth));

	grid(column + 1, 0, depth + 1) = (1.0f / 3.0f) * (grid(column, 0, depth + 1) + grid(column + 1, 1, depth + 1) + grid(column + 1, 0, depth));
	grid(column + 1, row + 1, depth + 1) = (1.0f / 3.0f) * (grid(column, row + 1, depth + 1) + grid(column + 1, row, depth + 1) + grid(column + 1, row + 1, depth));
}

void StableFluids::lin_solve(int b, GridStructure<float>& grid, GridStructure<float>& prevGrid, float a, float c)
{
	c = 1.0f / c;

	for(int t = 0; t < MAX_ITERATIONS; t++)
	{
		for(int i = 1; i <= column; i++)
		{
			for(int j = 1; j <= row; j++)
			{
				for(int k = 1; k <= depth; k++)
				{
					grid(i, j, k) = (prevGrid(i, j, k) + a * (grid(i - 1, j, k) + grid(i + 1, j, k) 
						+ grid(i, j - 1, k) + grid(i, j + 1, k) 
						+ grid(i, j, k - 1) + grid(i, j, k + 1))) * c;
				}
			}
		}
		set_boundary(b, grid);
	}
}

void StableFluids::diffuse(int b, GridStructure<float>& grid, GridStructure<float>& prevGrid, float diffRate, float timeStep)
{
	float scale = timeStep * diffRate * N * N * N;
	lin_solve(b, grid, prevGrid, scale, 1 + 6 * scale);
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

void StableFluids::project(GridStructure<float>& u, GridStructure<float>& v, GridStructure<float>& w, GridStructure<float>& p, GridStructure<float>& div)
{
	int i, j, k;
	float h = 1.0f / N;

	for(i = 1; i <= column; i++)
	{
		for(j = 1; j <= row; j++)
		{
			for(k = 1; k <= depth; k++)
			{
				div(i, j, k) = -(1.0f / 3.0f) * h * ((u(i + 1, j, k) - u(i - 1, j, k)) + (v(i, j + 1, k) - v(i, j - 1, k)) + (w(i, j, k + 1) - w(i, j, k - 1)));
				p(i, j, k) = 0;
			}
		}
	}

	set_boundary(0, div); 
	set_boundary(0, p);

	lin_solve(0, p, div, 1, 6);

	for(i = 1; i <= column; i++)
	{
		for(j = 1; j <= row; j++)
		{
			for(k = 1; k <= depth; k++)
			{
				u(i, j, k) -= 0.5f * N * (p(i + 1, j, k) - p(i - 1, j, k));
				v(i, j, k) -= 0.5f * N * (p(i, j + 1, k) - p(i, j - 1, k));
				w(i, j, k) -= 0.5f * N * (p(i, j, k + 1) - p(i, j, k - 1));
			}
		}
	}

	set_boundary(1, u);
	set_boundary(2, v);
	set_boundary(3, w);
}

void StableFluids::Simulate(float timeStep, float diffRate, bool& addForceU, bool& addForceV, bool& addForceW, bool& negaddForceU, bool& negaddForceV, bool& negaddForceW, bool& addSmoke)
{
	prevUVelocity.fill(0.f);
	prevVVelocity.fill(0.f);
	prevWVelocity.fill(0.f);
	prevSmoke.fill(0.f);

	if(addForceU)
	{
		prevUVelocity(2, row / 2, depth / 2) = 100.f;
		addForceU = false;
	}

	if(addForceV)
	{
		prevVVelocity(column / 2, 2, depth / 2) = 200.f;
		addForceV = false;
	}

	if(addForceW)
	{
		prevWVelocity(column / 2, row / 2, 2) = 200.f;
		addForceW = false;
	}

	if(negaddForceU)
	{
		prevUVelocity(column - 2, row / 2, depth / 2) = -200.f;
		negaddForceU = false;
	}

	if(negaddForceV)
	{
		prevVVelocity(column / 2, row - 2, depth / 2) = -200.f;
		negaddForceV = false;
	}

	if(negaddForceW)
	{
		prevWVelocity(column / 2, row / 2, depth - 2) = -200.f;
		negaddForceW = false;
	}

	if(addSmoke)
	{
		prevSmoke(column / 2, 2, depth / 2) = 100.f;
		addSmoke = false;
	}

	velocity_step(&uVelocity, &vVelocity, &wVelocity, &prevUVelocity, &prevVVelocity, &prevWVelocity, timeStep);
	density_step(&smoke, &prevSmoke, diffRate, timeStep);
}

void StableFluids::density_step(GridStructure<float>* grid, GridStructure<float>* prevGrid, float diffRate, float timeStep)
{
	add_source(*grid, *prevGrid, timeStep);
	SWAP(prevGrid, grid); 
	diffuse(0, *grid, *prevGrid, diffRate, timeStep);
	SWAP(prevGrid, grid); 
	advect(0, *grid, *prevGrid, uVelocity, vVelocity, wVelocity, timeStep);
}

void StableFluids::velocity_step(GridStructure<float>* u, GridStructure<float>* v, GridStructure<float>* w, GridStructure<float>* u0, GridStructure<float>* v0, GridStructure<float>* w0, float timeStep)
{
	add_source(*u, *u0, timeStep); 
	add_source(*v, *v0, timeStep); 
	add_source(*w, *w0, timeStep);

	SWAP(u0, u); 
	diffuse(1, *u, *u0, viscosity, timeStep);
	SWAP(v0, v); 
	diffuse(2, *v, *v0, viscosity, timeStep);
	SWAP(w0, w); 
	diffuse(3, *w, *w0, viscosity, timeStep);

	project(*u, *v, *w, *u0, *v0);

	SWAP(u0, u); 
	SWAP(v0, v); 
	SWAP(w0, w);

	advect(1, *u, *u0, *u0, *v0, *w0, timeStep); 
	advect(2, *v, *v0, *u0, *v0, *w0, timeStep); 
	advect(3, *w, *w0, *u0, *v0, *w0, timeStep);

	project(*u, *v, *w, *u0, *v0);
}
