#pragma once
#include "WorldObject.h"
#include "../renderer/Mesh.h"

class RenderedObject : public WorldObject
{
public:
	Mesh mesh;

	RenderedObject(const Mesh& mesh) : mesh(mesh) {};
	RenderedObject(const Mesh& mesh, const Vector3f& location) : mesh(mesh), WorldObject(location) {};

	void DrawObject(const Matrix4f& cameraMatrix)
	{
		mesh.Draw(cameraMatrix, location, rotation, scale);
	}
};

