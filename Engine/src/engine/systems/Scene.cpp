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

enum class TypesMap
{
	b = 0,
	flt = 1,
	i8 = 2,
	i16 = 3,
	i32 = 4,
	str = 5
};

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
			LOGERRORF("JSON parse error at {} {}", e.byte, e.what());
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
			auto newcircle = scene.AddToScene(Circle(circle.key()));
			auto& c = dynamic_cast<Circle*>(newcircle)->GetCircle();

			auto& props = circle.value()["circle_properties"];
			auto x = props["posx"].get<float>();
			auto y = props["posy"].get<float>();

			c.setPosition({ x, y });
		
			for (const auto& script : circle.value()["scripts"].items())
			{
				auto gscripts = Engine::Get().GetGameScriptsRegister();
				if (auto it = gscripts.find(script.key()); it != gscripts.end())
				{
					newcircle->AddScript(it->second->Clone());
					auto new_attached_script = newcircle->GetScript(it->first);
					auto& vars = new_attached_script->GetReflection().Get();
					int i = 0;
					for (const auto& script_vars : script.value().items())
					{
						switch(i++)
						{
						case (int)TypesMap::b:
							for (const auto& i : script_vars.value().items())
							{
								*vars.b[i.key()] = i.value().get<bool>();
							}
							break;
						case (int)TypesMap::flt:
							for (const auto& i : script_vars.value().items())
							{
								*vars.flt[i.key()] = i.value().get<float>();
							}
							break;
						case (int)TypesMap::i8:
							for (const auto& i : script_vars.value().items())
							{
								*vars.i8[i.key()] = i.value().get<int8_t>();
							}
							break;
						case (int)TypesMap::i16:
							for (const auto& i : script_vars.value().items())
							{
								*vars.i16[i.key()] = i.value().get<int16_t>();
							}
							break;
						case (int)TypesMap::i32:
							for (const auto& i : script_vars.value().items())
							{
								*vars.i32[i.key()] = i.value().get<int32_t>();
							}
							break;
						case (int)TypesMap::str:
							for (const auto& s : script_vars.value().items())
							{
								*vars.str[s.key()] = s.value().get<std::string>();
							}
							break;
						default: 
							LOGWARNF("Unknown type for script_vars iteration: {} key: {} value: {}", i, script_vars.key(), script_vars.value());
							break;
						}
					}
				}
                else
                {
                    LOGWARNF("Script not found needed by scene with name: {}", script.key());
                }
			}
		}
	}

	return scene;
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
