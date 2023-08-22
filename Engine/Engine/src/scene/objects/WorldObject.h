#pragma once
#include <set>
#include <unordered_set>
#include <memory>
#include "../../math/Vectorf.h"
#include "components/static/TransformComponent.h"
#include "components/active/ActiveComponent.h"
#include "../../renderer/Mesh.h"

class WorldObject
{
private:
	void CopyActiveComponents(const WorldObject& other);
public:

	TransformComponent transform;
	std::unordered_set<std::unique_ptr<ActiveComponent>> activeComponents;

	WorldObject(const TransformComponent transform = TransformComponent()) : transform(transform) {};

	WorldObject(const WorldObject& other);
	WorldObject(WorldObject&& source) noexcept;
	WorldObject& operator=(const WorldObject& other);

	template<class T>
	inline void AddComponent(const T& component)
	{
		static_assert(std::is_convertible<T*, ActiveComponent*>::value, "Class must inherit ActiveComponent");
		activeComponents.emplace(std::make_unique<T>(component));
	}

	inline void FrameUpdateActiveComponents() const
	{
		for(const std::unique_ptr<ActiveComponent>& component : activeComponents)
		{
			component->FrameUpdate();
		}
	}
};

WorldObject CreateModel(const Mesh& model, const TransformComponent& transform = TransformComponent());