#pragma once
#include "../../../opencl/opencl.hpp"
#include "../../../renderer/shaders/LineShader.h"

class OpenCLFluids
{
private:
	unsigned int column, row, depth, N;

	float viscosity;

	Device device;

	Kernel add_vel_sources, add_smoke_source;
	Kernel project1, linsolve, project2, advect;

	Kernel sidesBoundaryFace, topBottomBoundaryFace, frontBackBoundaryFace, boundaryIEdge, boundaryJEdge, boundaryKEdge, boundaryCorners;

	Kernel vel_step, dens_step;

	cl::CommandQueue cl_queue;
	cl::CommandQueue device_queue;

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

	void density_step(float timeStep, float diffRate);
	void velocity_step(float timeStep);

	cl::vector<Event> enqueue_set_boundary(Memory<float>& grid, const int f1, const int f2, const int f3, const cl::vector<Event>& waitEvent);
	std::shared_ptr<LineShader> lineShader;
	GLuint VAO, VBO, textureID;

	std::array<float, 6> vertices = { 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f };

public:
	Memory<float> smoke;

	OpenCLFluids(unsigned int _column, unsigned int _row, unsigned int _depth);

	void Simulate(float timeStep, float diffRate, bool& addForceU, bool& addForceV, bool& addForceW, bool& negAddForceU, bool& negAddForceV, bool& negAddForceW, bool& addSmoke, bool& clear,
		float xForce, float yForce);

	void InitVelocityRender();
	void VelocityRender(Matrix4f& cameraMatrix);
};