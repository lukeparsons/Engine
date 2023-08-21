#include "WorldObject.h"
#include <cassert>
#include <cstdint>
#include "components/active/RenderComponent.h"
#include "../Scene.h"

WorldObject::~WorldObject()
{
	// TODO: Delete children as well here
	if(parentObject != nullptr)
	{
		parentObject->RemoveChildFromList(*this);
	}
}

void WorldObject::CopyActiveComponents(const WorldObject& other)
{
	for(const std::unique_ptr<ActiveComponent>& component : other.activeComponents)
	{
		activeComponents.insert(std::move(component.get()->Clone(*this)));
	}
}

WorldObject::WorldObject(const WorldObject& other) : transform(other.transform), parentObject(other.parentObject)
{
	CopyActiveComponents(other);
}

WorldObject::WorldObject(const WorldObject& other, Scene& scene) : transform(other.transform), parentObject(other.parentObject)
{
	CopyActiveComponents(other);

	std::set<std::weak_ptr<WorldObject>, std::owner_less<std::weak_ptr<WorldObject>>>::iterator it;
	std::set<std::weak_ptr<WorldObject>, std::owner_less<std::weak_ptr<WorldObject>>> otherChildList = other.GetChildren();
	
	for(it = otherChildList.begin(); it != otherChildList.end(); ++it)
	{
		std::shared_ptr childSP = (*it).lock();
		if(childSP)
		{
			WorldObject copyChild(*childSP.get());
			copyChild.parentObject = this;
			AddChildInScene(scene, std::make_shared<WorldObject>(copyChild));
		}
	} 
}

WorldObject::WorldObject(WorldObject&& source) noexcept
{
	childList = std::move(source.childList);
	source.childList.reset();
	parentObject = std::move(source.parentObject);
	source.parentObject = nullptr;
	transform = source.transform;
	activeComponents = std::move(source.activeComponents);
}

WorldObject& WorldObject::operator=(const WorldObject& other)
{
	transform = other.transform;
	CopyActiveComponents(other);

	return *this;
}

void WorldObject::PreInsertChildInScene(const Scene& scene, std::shared_ptr<WorldObject>& child)
{
	assert(child.get()->parentObject == nullptr);
	assert(scene.GetWorldObjects().contains(child));

	if(childList == nullptr)
	{
		childList = std::make_unique<std::set<std::weak_ptr<WorldObject>, std::owner_less<std::weak_ptr<WorldObject>>>>();
	}
	child.get()->parentObject = this;
}

void WorldObject::AddChildInScene(Scene& scene, std::shared_ptr<WorldObject>& child)
{
	PreInsertChildInScene(scene, child);
	assert(childList.get()->insert(std::weak_ptr<WorldObject>(child)).second == false);
}

void WorldObject::AddChildInScene(Scene& scene, std::shared_ptr<WorldObject>&& child)
{
	PreInsertChildInScene(scene, child);
	assert(childList.get()->emplace(std::move(std::weak_ptr<WorldObject>(child))).second == false);
}

void WorldObject::RemoveChildFromList(std::shared_ptr<WorldObject>& child)
{
	if(childList != nullptr)
	{
		// TODO: One liner (.second)
		if(childList.get()->contains(child))
		{
			childList.get()->erase(child);
			child.get()->parentObject = nullptr;
		}
	}
}

void WorldObject::RemoveChildFromList(const WorldObject& child)
{
	std::shared_ptr<WorldObject> childSP = std::make_shared<WorldObject>(child);
	RemoveChildFromList(childSP);
}

WorldObject CreateModel(const Mesh& model, const TransformComponent& transform)
{
	WorldObject newObject =WorldObject(transform);
	RenderComponent renderComponent = RenderComponent(&model, &newObject.transform);
	newObject.AddComponent(renderComponent);
	return newObject;
}