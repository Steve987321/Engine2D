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

//Object* Scene::AddToScene(Circle&& obj)
//{
//	auto objName = obj.name;
//	if (objectsMap.contains(objName))
//	{
//		objName += " (" + std::to_string(objectsMap.count(objName)) + ')';
//		while (objectsMap.contains(objName))
//		{
//			objName += " (" + std::to_string(objectsMap.count(objName)) + ')';
//		}
//	}
//
//	obj.name = objName;
//	objectsMap.insert({objName, std::make_shared<Circle>(obj)});
//	return objectsMap[objName].get();
//}
//
//Object* Scene::AddToScene(Sprite&& obj)
//{
//	auto objName = obj.name;
//	if (objectsMap.contains(objName))
//	{
//		objName += " (" + std::to_string(objectsMap.count(objName)) + ')';
//		while (objectsMap.contains(objName))
//		{
//			objName += " (" + std::to_string(objectsMap.count(objName)) + ')';
//		}
//	}
//
//	obj.name = objName;
//	objectsMap.insert({ objName, std::make_shared<Sprite>(obj) });
//	return objectsMap[objName].get();
//}

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
