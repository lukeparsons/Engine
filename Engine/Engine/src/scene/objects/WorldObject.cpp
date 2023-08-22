#include "WorldObject.h"
#include <cassert>
#include <cstdint>
#include "components/active/RenderComponent.h"
#include "../Scene.h"

void WorldObject::CopyActiveComponents(const WorldObject& other)
{
	for(const std::unique_ptr<ActiveComponent>& component : other.activeComponents)
	{
		activeComponents.insert(std::move(component.get()->Clone(*this)));
	}
}

WorldObject::WorldObject(const WorldObject& other) : transform(other.transform)
{
	CopyActiveComponents(other);
}

WorldObject::WorldObject(WorldObject&& source) noexcept
{
	transform = source.transform;
	activeComponents = std::move(source.activeComponents);
}

WorldObject& WorldObject::operator=(const WorldObject& other)
{
	transform = other.transform;
	CopyActiveComponents(other);

	return *this;
}

WorldObject CreateModel(const Mesh& model, const TransformComponent& transform)
{
	WorldObject newObject =WorldObject(transform);
	RenderComponent renderComponent = RenderComponent(&model, &newObject.transform);
	newObject.AddComponent(renderComponent);
	return newObject;
}