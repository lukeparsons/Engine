#pragma once
#include <set>
#include <unordered_set>
#include <memory>
#include "../../math/Vectorf.h"
#include "components/static/TransformComponent.h"
#include "components/active/ActiveComponent.h"
#include "../../renderer/Mesh.h"

class Scene;

class WorldObject
{
private:
	std::unique_ptr<std::set<std::weak_ptr<WorldObject>, std::owner_less<std::weak_ptr<WorldObject>>>> childList = nullptr;
	WorldObject* parentObject = nullptr;

	void PreInsertChildInScene(const Scene& scene, std::shared_ptr<WorldObject>& child);
	void CopyActiveComponents(const WorldObject& other);
public:

	TransformComponent transform;
	std::unordered_set<std::unique_ptr<ActiveComponent>> activeComponents;

	WorldObject(const TransformComponent transform = TransformComponent()) : transform(transform) {};
	~WorldObject();

	WorldObject(const WorldObject& other);
	WorldObject(const WorldObject& other, Scene& scene);
	WorldObject(WorldObject&& source) noexcept;
	WorldObject& operator=(const WorldObject& other);

	template<class T>
	inline void AddComponent(const T& component)
	{
		static_assert(std::is_convertible<T*, ActiveComponent*>::value, "Class must inherit ActiveComponent");
		activeComponents.emplace(std::make_unique<T>(component));
	}

	void AddChildInScene(Scene& scene, std::shared_ptr<WorldObject>& child);
	void AddChildInScene(Scene& scene, std::shared_ptr<WorldObject>&& child);
	void RemoveChildFromList(std::shared_ptr<WorldObject>& child);
	void RemoveChildFromList(const WorldObject& child);

	inline void FrameUpdateActiveComponents() const
	{
		for(const std::unique_ptr<ActiveComponent>& component : activeComponents)
		{
			component->FrameUpdate();
		}
	}

	const std::set<std::weak_ptr<WorldObject>, std::owner_less<std::weak_ptr<WorldObject>>>& GetChildren() const
	{
		return *childList.get();
	}
};

WorldObject CreateModel(const Mesh& model, const TransformComponent& transform = TransformComponent());