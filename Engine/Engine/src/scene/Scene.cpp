#include "Scene.h"
#include <cassert>
#include <algorithm>

void Scene::AddWorldObject(const WorldObject& worldObject)
{
	if(worldObject.activeComponents.size() > 0)
	{
		numberActiveObjects++;

	}
	worldObjects.insert(std::make_shared<WorldObject>(worldObject));
}

void Scene::DeleteWorldObject(std::shared_ptr<WorldObject>& worldObject)
{ 
	assert(worldObjects.contains(worldObject));
	worldObjects.erase(worldObject);
	worldObject.reset();
}

/*void Scene::AddWorldObject(WorldObject&& worldObject)
{
	if(worldObject.activeComponents.size() > 0)
	{
		numberActiveObjects++;
	}
	worldObjects.insert(std::move(worldObject));
} */


void Scene::FrameUpdateActiveObjects() const
{
	int i = 0;
	std::multiset<std::shared_ptr<WorldObject>, ComponentCompare>::iterator it = worldObjects.begin();
	while(i < numberActiveObjects)
	{
		(*it).get()->FrameUpdateActiveComponents();
		it = std::next(it, 1);
		i++;
	}
}