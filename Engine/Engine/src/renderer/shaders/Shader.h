#pragma once
#include <glad/glad.h>
#include "../../types/Maybe.h"

class Shader
{
private:

	Maybe<GLuint> id;

public:

	Shader(const char* path);
	~Shader();

	inline Maybe<GLuint> GetID() const { return id; }
};
