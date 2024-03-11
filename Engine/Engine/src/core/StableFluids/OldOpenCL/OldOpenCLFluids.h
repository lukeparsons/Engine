#pragma once
#include "../../../opencl/opencl.hpp"
#include "../../../renderer/shaders/LineShader.h"
#include "../Fluid.h"

class OldOpenCLFluids : public Fluid
{
private:

	Kernel addSource, project1, linsolve, project2, advect;
	Kernel sidesBoundaryFace, topBottomBoundaryFace, frontBackBoundaryFace, boundaryIEdge, boundaryJEdge, boundaryKEdge, boundaryCorners;

	cl::CommandQueue cl_queue;

	Memory<float> uVelocity;
	Memory<float> vVelocity;
	Memory<float> wVelocity;
	Memory<float> smoke;
	Memory<float> prevUVelocity;
	Memory<float> prevVVelocity;
	Memory<float> prevWVelocity;
	Memory<float> prevSmoke;

	float scale;

	inline void read(Memory<float>& grid, unsigned int column, unsigned int row, unsigned int depth)
	{
		grid.read_from_device_3d(0, column, 0, row, 0, depth, column, row, depth);
	}

	inline void write(Memory<float>& grid, unsigned int column, unsigned int row, unsigned int depth)
	{
		grid.write_to_device_3d(0, column, 0, row, 0, depth, column, row, depth);
	}

	inline void read_all(Memory<float>& grid)
	{
		read(grid, column + 2, row + 2, depth + 2);
	}

	inline void write_all(Memory<float>& grid)
	{
		write(grid, column + 2, row + 2, depth + 2);
	}

	inline void write_centre(Memory<float>& grid)
	{
		grid.write_to_device_3d(0, column, 0, row, 0, depth, column, row, depth);
	}

	inline void read_centre(Memory<float>& grid)
	{
		grid.read_from_device_3d(0, column, 0, row, 0, depth, column, row, depth);
	}

	void project(Memory<float>& u, Memory<float>& v, Memory<float>& w, Memory<float>& u0, Memory<float>& v0);
	void lin_solve(const int b, Memory<float>& grid);

	void density_step(float timeStep);
	void velocity_step(float timeStep);

	void set_boundary(int b, Memory<float>& grid);

	std::shared_ptr<LineShader> lineShader;
	GLuint VAO, VBO, textureID;

	std::array<float, 6> vertices = { 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f };

public:

	float* GetSmokeData()
	{
		return smoke.data();
	}

	OldOpenCLFluids(unsigned int _column, unsigned int _row, unsigned int _depth, int iterations = 20);

	void Simulate(float timeStep, bool& addForceU, bool& addForceV, bool& addForceW, bool& negAddForceU, bool& negAddForceV, bool& negAddForceW, bool& addSmoke, bool& clear);

	void Profile(float timeStep, float addForceU, float addForceV, float addForceW, float negAddForceU, float negAddForceV, float negAddForceW, float addSmoke);
};