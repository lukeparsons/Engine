#pragma once
#include <set>
#include "objects/WorldObject.h"

struct ComponentCompare
{
	bool operator()(const WorldObject* lhs, const WorldObject* rhs) const
	{
		size_t lhsActiveSize = lhs->activeComponents.size();
		size_t rhsActiveSize = rhs->activeComponents.size();
		return lhsActiveSize > rhsActiveSize;
	}
};

class Scene
{
private:
	size_t numberActiveObjects = 0;
	std::multiset<WorldObject*, ComponentCompare> worldObjects;
public:

	~Scene();

	void AddWorldObject(WorldObject *const worldObject);

	void DeleteWorldObject(WorldObject *const worldObject);

	void FrameUpdateActiveComponents() const;
};

