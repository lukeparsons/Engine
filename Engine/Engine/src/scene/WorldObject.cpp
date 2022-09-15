#include "WorldObject.h"

WorldObject::WorldObject(Vector3f location)
{
	this->location = location;
}

WorldObject::WorldObject(Vector3f location, float vertices[])
{
	this->location = location;
	this->vertices = vertices;
}
