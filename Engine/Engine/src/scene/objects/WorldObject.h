#pragma once
#include <unordered_set>
#include <memory>
#include "../../types/Maybe.h"
#include "../../math/Vectorf.h"
#include "components/static/TransformComponent.h"
#include "components/active/ActiveComponent.h"
#include "../../renderer/Mesh.h"

class WorldObject
{
private:
	std::unique_ptr<std::unordered_set<WorldObject*>> childList = nullptr;
public:
	Maybe<WorldObject*> parentObject = Maybe<WorldObject*>(Nothing());
	TransformComponent transform;
	std::vector<ActiveComponent*> activeComponents;

	WorldObject() : transform(TransformComponent()) {};
	WorldObject(const TransformComponent transform) : transform(transform) {};

	~WorldObject();

	WorldObject(const WorldObject& other);

	void AddComponent(ActiveComponent *const component)
	{
		activeComponents.push_back(component);
	}

	void AddChild(WorldObject* child);
	void DeleteChildFromList(WorldObject* child);

	void FrameUpdateActiveComponents()
	{
		for(ActiveComponent* component : activeComponents)
		{
			component->FrameUpdate();
		}
	}

	std::unordered_set<WorldObject*>& GetChildren() const
	{
		return *childList.get();
	}
};

WorldObject* CreateModel(const Mesh& model, const TransformComponent& transform = TransformComponent());