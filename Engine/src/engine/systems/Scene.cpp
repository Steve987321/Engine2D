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
	for (auto& obj : objects_map | std::views::values)
	{
		obj->Start();
	}
}

void Scene::Update()
{
	for (auto& obj : objects_map | std::views::values)
	{
		obj->Update();
	}
}

void Scene::Render(sf::RenderTexture& texture)
{
	for (auto& obj : objects_map | std::views::values)
	{
		obj->Render(texture);
	}
}

void Scene::Render(sf::RenderWindow& window)
{
	for (auto& obj : objects_map | std::views::values)
	{
		obj->Render(window);
	}
}


Object* Scene::GetSceneObject(std::string_view obj_name)
{
	if (objects_map.contains(obj_name.data()))
	{
		return objects_map[obj_name.data()].get();
	}
	return nullptr;
}

bool Scene::RemoveFromScene(std::string_view obj_name)
{
	if (objects_map.contains(obj_name.data()))
	{
		objects_map.erase(obj_name.data());
		return true;
	}
	
	return false;
}

}
