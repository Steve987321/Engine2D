#include "pch.h"
#include "Scene.h"

#include <ranges>

#include "engine/object_default/Object.h"

namespace Toad
{

void Scene::Start()
{
	for (auto& obj : objectsMap | std::views::values)
	{
		obj.Start();
	}
}

void Scene::Update(sf::RenderWindow& window)
{
	for (auto& obj : objectsMap | std::views::values)
	{
		obj.Update(window);
	}
}

Object* Scene::GetSceneObject(std::string_view obj_name)
{
	if (objectsMap.contains(obj_name.data()))
	{
		return &objectsMap[obj_name.data()];
	}
	return nullptr;
}

Object* Scene::AddToScene(const Object& obj)
{
	auto objName = obj.name;
	if (objectsMap.contains(objName))
	{
		objName += " (" + std::to_string(objectsMap.count(objName)) + ')';
		while (objectsMap.contains(objName))
		{
			objName += " (" + std::to_string(objectsMap.count(objName)) + ')';
		}
	}

	objectsMap.insert({objName, obj});
	return &objectsMap[objName];
}

bool Scene::RemoveFromScene(std::string_view obj_name)
{
	if (objectsMap.contains(obj_name.data()))
	{
		objectsMap.erase(obj_name.data());
		return true;
	}
	
	return false;
}

}
