#pragma once
#include "../../../opencl/opencl.hpp"
#include "../../../renderer/shaders/LineShader.h"
#include "../Fluid.h"

class OpenCLFluids : public Fluid
{
private:

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

	void density_step(float timeStep);
	void velocity_step(float timeStep);

	cl::vector<Event> enqueue_set_boundary(Memory<float>& grid, const int f1, const int f2, const int f3, const cl::vector<Event>& waitEvent);
	std::shared_ptr<LineShader> lineShader;
	GLuint VAO, VBO, textureID;

	std::array<float, 6> vertices = { 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f };

	cl::Event project_event;
	cl::Event solveP_event;
	cl::Event project2_event;
	cl::Event solveU_event, solveV_event, solveW_event;
	cl::Event advect_event;
	cl::Event linsolve_event;
	cl::Event source_event;
	cl::Event advect_u, advect_v, advect_w;
	cl::Event frontBack, topBottom, sidesBoundary, boundaryI, boundaryJ, boundaryK, boundaryCorner;
	cl::vector<Event> advect_bnd_u, advect_bnd_v, advect_bnd_w;
	cl::vector<Event> bnd_event;
	cl::vector<Event> uBnd, vBnd, wBnd;
	cl::vector<Event> div_bnd_event, p_bnd_event;

public:

	OpenCLFluids(unsigned int _column, unsigned int _row, unsigned int _depth, int iterations = 20);

	void Simulate(float timeStep, bool& addForceU, bool& addForceV, bool& addForceW, bool& negAddForceU, bool& negAddForceV, bool& negAddForceW, bool& addSmoke, bool& clear);

	void Profile(float timeStep, float addForceU, float addForceV, float addForceW, float negAddForceU, float negAddForceV, float negAddForceW, float addSmoke);

	float* GetSmokeData()
	{
		return smoke.data();
	}

	void InitVelocityRender();
	void VelocityRender(Matrix4f& cameraMatrix);
};