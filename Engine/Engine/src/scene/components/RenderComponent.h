#pragma once

#include "../../renderer/Mesh.h"
#include "TransformComponent.h"

class RenderComponent : public Component {
private:
	std::shared_ptr<Texture> texture;
public:
	std::shared_ptr<Mesh> mesh;
	GLuint textureID;

	RenderComponent(EntityID _id);

	void ChangeTexture(const Texture& _texture);
	void ChangeTexture(std::shared_ptr<Texture> _texture);
};