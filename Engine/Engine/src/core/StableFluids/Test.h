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
#define SIZE 24 // Best not to raise this very high

extern void dens_step(int M, int N, int O, float* x, float* x0, float* u, float* v, float* w, float diff, float dt);
extern void vel_step(int M, int N, int O, float* u, float* v, float* w, float* u0, float* v0, float* w0, float visc, float dt);

static int M = SIZE; // grid x
static int N = SIZE; // grid y
static int O = SIZE; // grid z
static float dt = 0.4f; // time delta
static float diff = 0.0f; // diffuse
static float visc = 0.0f; // viscosity
static float force = 10.0f;  // added on keypress on an axis
static float source = 200.0f; // density
static float source_alpha = 0.05; //for displaying density

static std::vector<std::shared_ptr<Line>> lines;


static float* u, * v, * w, * u_prev, * v_prev, * w_prev;
static float* dens, * dens_prev;


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

inline void initsim(Scene& scene)
{
	allocate_data();
	clear_data();
	for(int i = 1; i <= M; i++)
	{
		for(int j = 1; j <= N; j++)
		{
			for(int k = 1; k <= O; k++)
			{
				std::shared_ptr<Line> line = std::make_shared<Line>(0.25f, 0.1f, 0, 1.f, 1.f, 1.f);
				lines.push_back(line);
				EntityID id = scene.CreateLine(line);
				scene.GetComponent<RenderComponent>(id)->SetColour({ 0.5f, 0.8f, 0.1f });
			}
		}
	}


	u_prev[IX(2, N / 2, O / 2)] = 100.f;
}


inline void sim_main()
{
	for(int i = 0; i < (M + 2) * (N + 2) * (O + 2); i++)
	{
		u_prev[i] = v_prev[i] = w_prev[i] = dens_prev[i] = 0;
	}

	vel_step(M, N, O, u, v, w, u_prev, v_prev, w_prev, visc, dt);
	dens_step(M, N, O, dens, dens_prev, u, v, w, diff, dt);
}

inline void sim_draw()
{
	float h = 1.0f / N;
	float x, y, z;
	for(int i = 1; i <= M; i++)
	{
		x = (i - 0.5f) * h;
		for(int j = 1; j <= N; j++)
		{
			y = (j - 0.5f) * h;
			for(int k = 1; k <= O; k++)
			{
				z = (k - 0.5f) * h;
				lines[(i - 1) + M * ((j - 1) + N * (k - 1))]->ChangeLine(x, y, z, x + u[IX(i, j, k)], y + v[IX(i, j, k)], z + w[IX(i, j, k)]);
				//lines[(i - 1) + M * ((j - 1) + N * (k - 1))]->ChangeLine(i * 0.25f, 0.1f * j, 0.f * k, 1.f, 1.f, 1.f);
			}
		}
	}
}