#pragma once
#include "../math/Matrix4f.h"
#include <glad/glad.h>

class Drawable
{
public:
	virtual void Draw(const Matrix4f& cameraMatrix, GLuint textureID) const = 0;
};