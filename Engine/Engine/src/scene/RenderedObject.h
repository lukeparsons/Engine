#pragma once
#include "WorldObject.h"
#include "../renderer/Mesh.h"

class RenderedObject : public WorldObject
{
public:
	Mesh mesh;

	RenderedObject(const Mesh& mesh) : mesh(mesh) {};
	RenderedObject(const Mesh& mesh, const Vector3f& location) : mesh(mesh), WorldObject(location) {};
	RenderedObject(const Mesh& mesh, const Vector3f& location, const Vector3f& scale) : mesh(mesh), WorldObject(location, scale) {};

	void DrawObject(const Matrix4f& cameraMatrix)
	{
		mesh.Draw(cameraMatrix, location, rotation, scale);
	}
};

