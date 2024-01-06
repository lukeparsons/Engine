#include "RenderComponent.h"
#include "../../renderer/shaders/BasicShader.h"
#include "../../renderer/shaders/ShaderStore.h"

static const std::shared_ptr<Texture> defaultTexture = std::make_shared<TextureData<unsigned char>>(LoadPng("../Engine/assets/wall2.png"));

RenderComponent::RenderComponent(EntityID _id) : Component(_id), texture(defaultTexture), isActive(true)
{
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height, 0, texture->format, texture->type, texture->GetPixelData());

	glBindTexture(GL_TEXTURE_2D, 0);

	shaderProgram = g_shaderStore.LoadShader<BasicShader>("../Engine/src/renderer/shaders/shaderfiles/BasicVertex.vertex", "../Engine/src/renderer/shaders/shaderfiles/BasicFragment.fragment");
}

void RenderComponent::ChangeTexture(std::shared_ptr<Texture> _texture)
{
	if(texture == _texture)
	{
		return;
	}

	texture = _texture;
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, _texture->format, _texture->width, _texture->height, 0, _texture->format, _texture->type, _texture->GetPixelData());
	glBindTexture(GL_TEXTURE_2D, 0);
}
