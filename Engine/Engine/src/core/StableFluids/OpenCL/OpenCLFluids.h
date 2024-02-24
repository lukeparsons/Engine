#pragma once
#include "../../../opencl/opencl.hpp"
#include "../../../renderer/shaders/LineShader.h"

class OpenCLFluids
{
private:
	unsigned int column, row, depth, N;

	float viscosity;

	Kernel addSource, project1, linsolve, project2, advect;
	Kernel sidesBoundaryFace, topBottomBoundaryFace, frontBackBoundaryFace, boundaryIEdge, boundaryJEdge, boundaryKEdge, boundaryCorners;

	Memory<float> uVelocity;
	Memory<float> vVelocity;
	Memory<float> wVelocity;
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

	void density_step(float timeStep, float diffRate);
	void velocity_step(float timeStep);

	void set_boundary(int b, Memory<float>& grid);

	std::shared_ptr<LineShader> lineShader;
	GLuint VAO, VBO, textureID;

	std::array<float, 6> vertices = { 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f };

public:
	Memory<float> smoke;

	OpenCLFluids(unsigned int _column, unsigned int _row, unsigned int _depth);

	void Simulate(float timeStep, float diffRate, bool& addForceV, bool& addSmoke);

	void InitVelocityRender();
	void VelocityRender(Matrix4f& cameraMatrix);
};