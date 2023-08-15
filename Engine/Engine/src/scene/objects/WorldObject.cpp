#include "WorldObject.h"
#include <cassert>
#include <cstdint>
#include "components/active/RenderComponent.h"

WorldObject::WorldObject(const WorldObject& other)
{
	parentObject = other.parentObject;
	for(WorldObject* child : other.GetChildren())
	{
		WorldObject copyChild(*child);
		copyChild.parentObject = this;
		AddChild(&copyChild);
	} 
}

WorldObject::~WorldObject()
{
	if(parentObject.isJust())
	{
		parentObject.fromJust()->DeleteChildFromList(this);
	}

	if(childList != nullptr)
	{
		for(WorldObject* child : *childList.get())
		{
			child->~WorldObject();
		}
	}
} 

void WorldObject::AddChild(WorldObject* child)
{
	assert(child->parentObject.isNothing()); // , "Trying to add child that already has a parent");

	if(childList == nullptr)
	{
		childList = std::make_unique<std::unordered_set<WorldObject*>>();
	}
	assert(childList.get()->insert(child).second == false); // , "Trying to add child again to same object");
	child->parentObject = this;
}

void WorldObject::DeleteChildFromList(WorldObject* child)
{
	if(childList != nullptr)
	{
		assert(childList.get()->erase(child) != 0); // , "Trying to delete object not in scene");
	}
}

WorldObject* CreateModel(const Mesh& model, const TransformComponent& transform)
{
	WorldObject* newObject = new WorldObject(transform);
	RenderComponent* renderComponent = new RenderComponent(model, &newObject->transform);
	newObject->AddComponent(renderComponent);
	return newObject;
}