#pragma once
#include <corecrt_malloc.h>
#include <iostream>
#include "../../scene/Scene.h"
#include "../../renderer/Line.h"
#include <memory>

#define IX(i,j,k) ((i)+(M+2)*(j) + (M+2)*(N+2)*(k)) 
#define MAX(a,b)            (((a) > (b)) ? (a) : (b))

#define WINDOW_TITLE "Fluid"
#define WINDOW_WIDTH 768
#define WINDOW_HEIGHT 768
#define SIZE 42 // Best not to raise this very high

extern void dens_step(int M, int N, int O, float* x, float* x0, float* u, float* v, float* w, float diff, float dt);
extern void vel_step(int M, int N, int O, float* u, float* v, float* w, float* u0, float* v0, float* w0, float visc, float dt);

static int M = SIZE; // grid x
static int N = SIZE; // grid y
static int O = SIZE; // grid z
static float dt = 0.4f; // time delta
static float diff = 0.f; // diffuse
static float visc = 0.000f; // viscosity
static float force = 10.0f;  // added on keypress on an axis
static float source = 200.0f; // density
static float source_alpha = 0.05; //for displaying density


static float* u, * v, * w, * u_prev, * v_prev, * w_prev;
static float* dens, * dens_prev;

static GLuint VAO, VBO;

static std::shared_ptr<LineShader> lineShader;
static std::array<float, 6> vertices;


static void free_data(void)
{
	if(u) free(u);
	if(v) free(v);
	if(w) free(w);
	if(u_prev) free(u_prev);
	if(v_prev) free(v_prev);
	if(w_prev) free(w_prev);
	if(dens) free(dens);
	if(dens_prev) free(dens_prev);
}

static int allocate_data(void)
{
	int size = (M + 2) * (N + 2) * (O + 2);

	u = (float*)malloc(size * sizeof(float));
	v = (float*)malloc(size * sizeof(float));
	w = (float*)malloc(size * sizeof(float));
	u_prev = (float*)malloc(size * sizeof(float));
	v_prev = (float*)malloc(size * sizeof(float));
	w_prev = (float*)malloc(size * sizeof(float));
	dens = (float*)malloc(size * sizeof(float));
	dens_prev = (float*)malloc(size * sizeof(float));

	if(!u || !v || !w || !u_prev || !v_prev || !w_prev || !dens || !dens_prev) {
		return (0);
	}

	return 1;
}

static void clear_data()
{
	int i, size = (M + 2) * (N + 2) * (O + 2);

	for(i = 0; i < size; i++)
	{
		u[i] = v[i] = w[i] = u_prev[i] = v_prev[i] = w_prev[i] = dens[i] = dens_prev[i] = 0.0f;
	}
}

inline void initsim()
{
	allocate_data();
	clear_data();

	lineShader = g_shaderStore.LoadShader<LineShader>("../Engine/src/renderer/shaders/shaderfiles/Line.vertex", "../Engine/src/renderer/shaders/shaderfiles/Line.fragment");

	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);

	glUseProgram(lineShader->GetID());
	float colour[3] = { 0.0f, 0.9f, 0.1f };
	glUniform3fv(lineShader->colourLoc, 1, colour);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// Vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);


	u_prev[IX(2, N / 2, O / 2)] = 100.f;
}


inline void sim_main(bool& addForceU, bool& addForceV, bool& addForceW, bool& negaddForceU, bool& negaddForceV, bool& negaddForceW)
{
	for(int i = 0; i < (M + 2) * (N + 2) * (O + 2); i++)
	{
		u_prev[i] = v_prev[i] = w_prev[i] = dens_prev[i] = 0;
	}

	if(addForceU)
	{
		u_prev[IX(2, N / 2, O / 2)] = 200.f;
		addForceU = false;
	}

	if(addForceV)
	{
		v_prev[IX(M/2, 2, O / 2)] = 200.f;
		addForceV = false;
	}

	if(addForceW)
	{
		w_prev[IX(M / 2, N / 2, 2)] = 200.f;
		addForceW = false;
	}

	if (negaddForceU)
	{
		u_prev[IX(M - 2, N / 2, O / 2)] = -200.f;
		negaddForceU = false;
	}

	if (negaddForceV)
	{
		v_prev[IX(M / 2, N - 2, O / 2)] = -200.f;
		negaddForceV = false;
	}

	if (negaddForceW)
	{
		w_prev[IX(M / 2, N / 2, O - 2)] = -200.f;
		negaddForceW = false;
	}

	vel_step(M, N, O, u, v, w, u_prev, v_prev, w_prev, visc, dt);
	dens_step(M, N, O, dens, dens_prev, u, v, w, diff, dt);
}

inline void sim_draw(Matrix4f& cameraMatrix)
{
	float h = 1.0f / N;
	float x, y, z;
	float tol = 0.2f;
	float endX, endY, endZ;
	glUniformMatrix4fv(lineShader->transformLoc, 1, GL_TRUE, cameraMatrix.matrix[0]);

	for(int i = 1; i <= M; i++)
	{
		x = (i - 0.5f) * h;
		for(int j = 1; j <= N; j++)
		{
			y = (j - 0.5f) * h;
			for(int k = 1; k <= O; k++)
			{
				z = (k - 0.5f) * h;

				if (u[IX(i, j, k)] >= tol || u[IX(i, j, k)] <= -tol)
				{
					endX = x;
				} else
				{
					endX = x + u[IX(i, j, k)];
				}
				if (v[IX(i, j, k)] >= tol || v[IX(i, j, k)] <= -tol)
				{
					endY = y;
				} else
				{
					endY = y + v[IX(i, j, k)];
				}
				if (w[IX(i, j, k)] >= tol || w[IX(i, j, k)] <= -tol)
				{
					endZ = z;
				} else
				{
					endZ = z + w[IX(i, j, k)];
				}
				vertices = { x, y, z, endX, endY, endZ };
				glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

				glDrawArrays(GL_LINES, 0, 2);
			}
		}
	}
}