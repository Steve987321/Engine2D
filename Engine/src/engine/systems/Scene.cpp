#include "pch.h"
#include "Scene.h"

#include "engine/object_default/Object.h"
#include "engine/object_default/Sprite.h"
#include "engine/object_default/Circle.h"

#include "Engine/Engine.h"

#include "nlohmann/json.hpp"

namespace Toad
{

using json = nlohmann::json;

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

enum class TypesMap
{
	b = 0,
	flt = 1,
	i8 = 2,
	i16 = 3,
	i32 = 4,
	str = 5
};


sf::Texture GetTexFromPath(const std::filesystem::path& path)
{
	sf::Texture tex;
	if (!tex.loadFromFile(path.string()))
	{
		LOGERRORF("Failed to load texture asset: {}", path);
		return {};
	}
	return tex;
}

sf::IntRect GetRectFromJSON(json obj)
{
	return sf::IntRect{
		obj["left"].get<int>(),
		obj["top"].get<int>(),
		obj["width"].get<int>(),
		obj["heigth"].get<int>(),
	};
}

template <typename T> 
void LoadSceneObjects(json objects, Scene& scene)
{
	std::queue<std::pair<std::string, std::string>> set_object_parents_queue {};

	for (const auto& object : objects.items())
	{
		auto& props = object.value()["properties"];
		auto x = props["posx"].get<float>();
		auto y = props["posy"].get<float>();

		Object* newobj = scene.AddToScene(T(object.key()));
		Sprite* spriteobj = dynamic_cast<Sprite*>(newobj);
		Circle* circleobj = dynamic_cast<Circle*>(newobj);

		std::string parent_name = props["parent"].get<std::string>();
		if (!parent_name.empty())
		{
			Object* parent_obj = scene.GetSceneObject(parent_name);
			if (parent_obj != nullptr)
			{
				newobj->SetParent(parent_obj);
			}
			else
			{
				set_object_parents_queue.emplace(newobj->name, parent_name);
			}
		}

		if (circleobj != nullptr)
		{
			auto& circle = circleobj->GetCircle();

			// props 
			sf::Color fill_col = sf::Color(props["fill_col"].get<int>());
			sf::Color outline_col = sf::Color(props["outline_col"].get<int>());
			sf::Vector2f scale = { props["scalex"].get<float>(), props["scaley"].get<float>() };
			float radius = props["radius"].get<float>();
			bool has_texture = props["has_texture"].get<bool>();

			if (has_texture)
			{
				std::string path_str = props["texture_loc"].get<std::string>();
				sf::Texture tex = GetTexFromPath(std::filesystem::path(path_str));
				sf::IntRect tex_rect = GetRectFromJSON(props["texture_rect"]);
				sf::Texture* new_tex = Engine::Get().GetResourceManager().AddTexture(path_str, tex);

				circle.setTexture(new_tex);
				circle.setTextureRect(tex_rect);
			}
			circle.setFillColor(fill_col);
			circle.setOutlineColor(outline_col);
			circle.setScale(scale);
			circle.setRadius(radius);
			circle.setPosition({ x, y });
		}
		else if (spriteobj != nullptr)
		{
			auto& sprite = spriteobj->GetSprite();

			// props
			sf::Color fill_col = sf::Color(props["fill_col"].get<int>());
			sf::Vector2f scale = sf::Vector2f{ props["scalex"].get<float>(), props["scaley"].get<float>() };
			
			float rotation = props["rotation"].get<float>();
			bool has_texture = props["has_texture"].get<bool>();

			if (has_texture)
			{
				std::string path_str = props["texture_loc"].get<std::string>();
				sf::Texture tex = GetTexFromPath(std::filesystem::path(props["texture_loc"].get<std::string>()));
				sf::IntRect texrect = GetRectFromJSON(props["texture_rect"]);
				sf::Texture* new_tex = Engine::Get().GetResourceManager().AddTexture(path_str, tex);
				sprite.setTexture(*new_tex);
				sprite.setTextureRect(texrect);
			}

			sprite.setPosition({ x, y });
			sprite.setColor(fill_col);
			sprite.setRotation(rotation);
			sprite.setColor(fill_col);
			sprite.setScale(scale);
		}

		for (const auto& script : object.value()["scripts"].items())
		{
			auto gscripts = Engine::Get().GetGameScriptsRegister();
			if (gscripts.empty())
			{
				LOGWARN("Scripts register is empty");
			}
			if (auto it = gscripts.find(script.key()); it != gscripts.end())
			{
				newobj->AddScript(it->second->Clone());
				auto new_attached_script = newobj->GetScript(it->first);
				auto& vars = new_attached_script->GetReflection().Get();
				int i = 0;
				for (const auto& script_vars : script.value().items())
				{
					switch (i++)
					{
					case (int)TypesMap::b:
						for (const auto& j : script_vars.value().items())
						{
							*vars.b[j.key()] = j.value().get<bool>();
						}
						break;
					case (int)TypesMap::flt:
						for (const auto& j : script_vars.value().items())
						{
							*vars.flt[j.key()] = j.value().get<float>();
						}
						break;
					case (int)TypesMap::i8:
						for (const auto& j : script_vars.value().items())
						{
							*vars.i8[j.key()] = j.value().get<int8_t>();
						}
						break;
					case (int)TypesMap::i16:
						for (const auto& j : script_vars.value().items())
						{
							*vars.i16[j.key()] = j.value().get<int16_t>();
						}
						break;
					case (int)TypesMap::i32:
						for (const auto& j : script_vars.value().items())
						{
							*vars.i32[j.key()] = j.value().get<int32_t>();
						}
						break;
					case (int)TypesMap::str:
						for (const auto& j : script_vars.value().items())
						{
							*vars.str[j.key()] = j.value().get<std::string>();
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

	while (!set_object_parents_queue.empty())
	{
		const auto& [child, parent] = set_object_parents_queue.front();
		Object* child_obj = scene.GetSceneObject(child);
		Object* parent_obj = scene.GetSceneObject(parent);

		child_obj->SetParent(parent_obj);

		set_object_parents_queue.pop();
	}

}

Scene LoadScene(const std::filesystem::path& path)
{
	std::ifstream in(path);

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
	scene.name = path.filename().string();

	if (data.contains("objects"))
	{
		auto objects = data["objects"];
		LoadSceneObjects<Circle>(objects["circles"], scene);
		LoadSceneObjects<Sprite>(objects["sprites"], scene);
	}

	return scene;
}

void SaveScene(const Scene& scene, const std::filesystem::path& path)
{
	json data; 

	// object types
	json objects; // all 
	json circles;
	json sprites;

	for (const auto& [name, object] : scene.objects_map)
	{
		Circle* circle = dynamic_cast<Circle*>(object.get());
		Sprite* sprite = dynamic_cast<Sprite*>(object.get());

		if (circle != nullptr)
		{
			circles[name] = circle->Serialize();
		}
		else if (sprite != nullptr)
		{
			sprites[name] = sprite->Serialize();
		}

		objects["circles"] = circles;
		objects["sprites"] = sprites;
		//objects["..."] = ...
	}

	data["objects"] = objects;

	std::string dir = path.string();
	if (dir.find('\\') != std::string::npos && !dir.ends_with("\\"))
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
