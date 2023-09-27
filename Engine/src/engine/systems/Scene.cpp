#include "pch.h"
#include "Scene.h"

#include <ranges>

#include "engine/object_default/Object.h"
#include "engine/object_default/Sprite.h"
#include "engine/object_default/Circle.h"

#include "Engine/Engine.h"

#include "nlohmann/json.hpp"
#include <fstream>

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

using json = nlohmann::json;

Scene LoadScene(std::string_view path)
{
	std::ifstream in(path.data());

	json data;
	if (in.is_open())
	{
		try
		{
			data = json::parse(in);
			in.close();
		}
		catch (json::parse_error& e)
		{
			LOGERROR("JSON parse error at {} {}", e.byte, e.what());
			in.close();
			return {};
		}
	}

	Scene scene;

	if (data.contains("objects"))
	{
		auto objects = data["objects"];
		for (const auto& circle : objects["circles"].items())
		{
			Circle newcircle(circle.key());
			auto& c = newcircle.GetCircle();

			for (const auto& c_data : circle.value().items())
			{
				for (const auto& properties : c_data.value()["circle_properties"].items())
				{
					auto x = properties.value()["posx"].get<float>();
					auto y = properties.value()["posy"].get<float>();
					c.setPosition({ x, y });
				}
		
				for (const auto& script : c_data.value()["scripts"].items())
				{
					Engine::Get().GetGameScriptsRegister();
					// script.key
				}

				
			}
		}
	}
	return scene;
}

Scene LoadScene(const std::filesystem::path& path)
{
	return Scene();
}

void SaveScene(const Scene& scene, std::string_view path)
{
	json data; 

	// object types
	auto objects = json::object(); // all 
	auto circles = json::object();

	for (const auto& [name, object] : scene.objects_map)
	{
		// global
		auto attached_scripts = json::object();
		for (auto& it : object->GetAttachedScripts())
		{
			const auto& reflection_vars = it.second->GetReflection().Get();
			const auto& bs = reflection_vars.b;
			const auto& flts = reflection_vars.flt;
			const auto& i8s = reflection_vars.i8;
			const auto& i16s = reflection_vars.i16;
			const auto& i32s = reflection_vars.i32;
			const auto& strs = reflection_vars.str;

			auto bs_data = json::object();
			for (const auto& [name, val] : bs)
			{
				bs_data[name] = *val;
			}
			auto flts_data = json::object();
			for (const auto& [name, val] : flts)
			{
				flts_data[name] = *val;
			}
			auto i8s_data = json::object();
			for (const auto& [name, val] : i8s)
			{
				i8s_data[name] = *val;
			}
			auto i16s_data = json::object();
			for (const auto& [name, val] : i16s)
			{
				i16s_data[name] = *val;
			}
			auto i32s_data = json::object();
			for (const auto& [name, val] : i32s)
			{
				i32s_data[name] = *val;
			}
			auto strs_data = json::object();
			for (const auto& [name, val] : strs)
			{
				strs_data[name] = *val;
			}
			attached_scripts[it.first] =
			{
				bs_data,
				flts_data,
				i8s_data,
				i16s_data,
				i32s_data,
				strs_data
			};
		}

		// circles
		Circle* circle = dynamic_cast<Circle*>(object.get());
		if (circle != nullptr)
		{
			auto& c = circle->GetCircle();

			auto c_data = json::object();
			auto c_circle_properties = json::object();

			c_circle_properties["posx"] = c.getPosition().x;
			c_circle_properties["posy"] = c.getPosition().y;
			c_circle_properties["col"] = c.getFillColor().toInteger();
			c_circle_properties["radius"] = c.getRadius();

			c_data["circle_properties"] = c_circle_properties;
			c_data["scripts"] = attached_scripts;

			circles[name] = c_data;
		}

		objects["circles"] = circles;
		//objects["..."] = ...
	}

	data["objects"] = objects;

	auto dir = std::string(path);
	if (dir.find("\\") != std::string::npos && !dir.ends_with("\\"))
		dir += "\\";
	auto full = dir + scene.name + ".TSCENE";
	std::ofstream out(full);

	if (out.is_open())
	{
		out << data;
		out.close();
	}
	else if (out.fail())
	{
		throw "bitchass";
	}
}

}
