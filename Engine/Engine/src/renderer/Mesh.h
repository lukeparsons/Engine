#pragma once

#include <vector>
#include "shaders/ShaderProgram.h"
#include <glad/glad.h>
#include "../math/Matrix4f.h"
#include "../math/Vectorf.h"
#include <assimp/scene.h>
#include "Texture.h"

struct Vertex
{
	Vector3f vertex;
	Vector2f tex;
	Vector3f normal;

	Vertex(aiVector3D& aiVertex, aiVector3D& aiTex, aiVector3D& aiNormal) {
		vertex = Vector3f(aiVertex.x, aiVertex.y, aiVertex.z);
		tex = Vector2f(aiTex.x, aiTex.y);
		normal = Vector3f(aiNormal.x, aiNormal.y, aiNormal.z);
	};
};

class Mesh
{
private:
	GLuint VAO;

	void readmesh(const std::string fileName);
	void constructmesh();
public:

	Mesh(const std::string fileName);

	Mesh(const Mesh& other);

	virtual void Draw(const Matrix4f& cameraMatrix, GLuint textureID) const;
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};