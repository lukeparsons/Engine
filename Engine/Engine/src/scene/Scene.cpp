#include "Scene.h"
#include <cassert>
#include <algorithm>

Scene::~Scene()
{
	for(std::multiset<WorldObject*, ComponentCompare>::iterator it = worldObjects.begin(); it != worldObjects.end(); ++it)
	{
		(*it)->~WorldObject();
	}
}

void Scene::AddWorldObject(WorldObject *const worldObject)
{
	worldObjects.insert(worldObject);
	if(worldObject->activeComponents.size() > 0)
		numberActiveObjects++;
}

void Scene::DeleteWorldObject(WorldObject *const worldObject)
{
	assert(worldObjects.contains(worldObject)); // "Trying to delete object not in scene");
	worldObjects.erase(worldObject);
	worldObject->~WorldObject();
}

void Scene::FrameUpdateActiveComponents() const
{
	int i = 0;
	std::multiset<WorldObject*, ComponentCompare>::iterator it = worldObjects.begin();
	while(i < numberActiveObjects)
	{
		for(ActiveComponent* component : (*it)->activeComponents)
		{
			component->FrameUpdate();
		}
		it = std::next(it, 1);
		i++;
	}
}