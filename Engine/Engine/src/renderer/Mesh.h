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
	unsigned int transformLoc;
	unsigned int modelLoc;
	unsigned int textureLoc;

	void readmesh(const char* fileName);
	void constructmesh(ShaderProgram *const shaderProgram);
public:
	ShaderProgram* shaderProgram;

	Mesh(const char* fileName, ShaderProgram *const shaderProgram);

	Mesh(const Mesh& other);

	void Draw(const Matrix4f& cameraMatrix, GLuint textureID, const Vector3f& location, const Vector3f& rotation, const Vector3f& scale) const;
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	//void temp_changecolour(GLfloat colour[4]);
};