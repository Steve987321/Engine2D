#include "pch.h"
#include "Scene.h"

#include <ranges>

#include "engine/object_default/Object.h"
#include "engine/object_default/Sprite.h"
#include "engine/object_default/Circle.h"

namespace Toad
{

void Scene::Start()
{
	for (auto& obj : objectsMap | std::views::values)
	{
		obj->Start();
	}
}

void Scene::Update(sf::RenderWindow& window)
{
	for (auto& obj : objectsMap | std::views::values)
	{
		obj->Update(window);
	}
}

void Scene::Update(sf::RenderTexture& window)
{
	for (auto& obj : objectsMap | std::views::values)
	{
		obj->Update(window);
	}
}

Object* Scene::GetSceneObject(std::string_view obj_name)
{
	if (objectsMap.contains(obj_name.data()))
	{
		return objectsMap[obj_name.data()].get();
	}
	return nullptr;
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
