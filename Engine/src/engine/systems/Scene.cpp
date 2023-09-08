#include "pch.h"
#include "Scene.h"

#include "engine/object_default/Object.h"

namespace Toad
{
	void Scene::Start()
	{
		for (auto& obj : objects)
		{
			obj.Start();
		}
	}

	void Scene::Update()
	{
	}
}
