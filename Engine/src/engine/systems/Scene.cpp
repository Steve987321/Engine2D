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
void LoadSceneObjects(json objects, Scene& scene, const std::filesystem::path& asset_folder = {})
{
#ifdef TOAD_EDITOR
	assert(!asset_folder.empty() && "asset_folder argument should be used when in toad editor");
#endif

	std::queue<std::pair<std::string, std::string>> set_object_parents_queue {};

	for (const auto& object : objects.items())
	{
		try
		{
			auto& props = object.value()["properties"];
			auto x = props["posx"].get<float>();
			auto y = props["posy"].get<float>();

			Object* newobj = scene.AddToScene(T(object.key()));
			Sprite* spriteobj = dynamic_cast<Sprite*>(newobj);
			Circle* circleobj = dynamic_cast<Circle*>(newobj);
			Audio* audioobj = dynamic_cast<Audio*>(newobj);
			Text* textobj = dynamic_cast<Text*>(newobj);

			newobj->SetPosition({ x,y });

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

					sf::Texture* new_tex = Engine::Get().GetResourceManager().GetTexture(path_str);
					sf::IntRect tex_rect = GetRectFromJSON(props["texture_rect"]);

					if (new_tex == nullptr)
					{
#ifdef TOAD_EDITOR
						sf::Texture tex = GetTexFromPath(asset_folder / std::filesystem::path(props["texture_loc"].get<std::string>()));
#else
						sf::Texture tex = GetTexFromPath(std::filesystem::path(props["texture_loc"].get<std::string>()));
#endif
						new_tex = Engine::Get().GetResourceManager().AddTexture(path_str, tex);
					}
					circleobj->SetTexture(path_str, new_tex);
					circle.setTextureRect(tex_rect);
				}
				circle.setFillColor(fill_col);
				circle.setOutlineColor(outline_col);
				circle.setScale(scale);
				circle.setRadius(radius);
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

					sf::Texture* new_tex = Engine::Get().GetResourceManager().GetTexture(path_str);
					sf::IntRect tex_rect = GetRectFromJSON(props["texture_rect"]);

					if (new_tex == nullptr)
					{
#ifdef TOAD_EDITOR
						sf::Texture tex = GetTexFromPath(asset_folder / std::filesystem::path(props["texture_loc"].get<std::string>()));
#else
						sf::Texture tex = GetTexFromPath(std::filesystem::path(props["texture_loc"].get<std::string>()));
#endif
						new_tex = Engine::Get().GetResourceManager().AddTexture(path_str, tex);
					}
					spriteobj->SetTexture(path_str, new_tex);
					sprite.setTextureRect(tex_rect);
				}

				sprite.setColor(fill_col);
				sprite.setRotation(rotation);
				sprite.setColor(fill_col);
				sprite.setScale(scale);
			}
			else if (audioobj != nullptr)
			{
				float play_from_src = props["play_from_source"].get<float>();
				float volume = props["volume"].get<float>();
				float pitch = props["pitch"].get<float>();
				float spatial_x = props["audio_posx"].get<float>();
				float spatial_y = props["audio_posy"].get<float>();
				float spatial_z = props["audio_posz"].get<float>();

				audioobj->ShouldPlayFromSource(play_from_src);
				audioobj->SetVolume(volume);
				audioobj->SetPitch(pitch);
				audioobj->SetAudioPosition({spatial_x, spatial_y, spatial_z});

				if (props.contains("audio_source"))
				{
					json audio_source_data = props["audio_source"];

					std::filesystem::path full_path = audio_source_data["full_path"].get<std::string>();
					std::filesystem::path rel_path = audio_source_data["rel_path"].get<std::string>();
					bool valid_buf = audio_source_data["has_valid_buf"].get<bool>();

					AudioSource new_audio_source;
					new_audio_source.has_valid_buffer = valid_buf;
					new_audio_source.full_path = full_path;
					new_audio_source.relative_path = rel_path;

					if (valid_buf)
					{
						sf::SoundBuffer sb;
#ifdef TOAD_EDITOR
						if (!sb.loadFromFile(full_path.string()))
						{
							LOGERRORF("[Scene] Loading soundbuffer file from path {} failed", full_path);
						}
#else
						sb.loadFromFile(new_audio_source.relative_path.string());
#endif

						new_audio_source.sound_buffer = sf::SoundBuffer(sb);
					}

					AudioSource* managed_audio_source = Engine::Get().GetResourceManager().AddAudioSource(rel_path.string(), new_audio_source);
					audioobj->SetSource(managed_audio_source);
				}
			}
			else if (textobj != nullptr)
			{
				// props
				textobj->SetText(props["text"].get<std::string>());
				std::string font_loc = props["font_loc"].get<std::string>();

				if (font_loc == "Default")
				{
					sf::Font* font = Engine::Get().GetResourceManager().GetFont("Default");
					if (font != nullptr)
					{
						textobj->SetFont("Default", *font);
					}
					else
					{
						sf::Font arial;

						if (!arial.loadFromFile("C:\\Windows\\Fonts\\Arial.ttf"))
						{
							LOGWARNF("Can't find C:\\Windows\\Fonts\\Arial.ttf");
						}
						else
						{
							font = Engine::Get().GetResourceManager().AddFont("Default", arial);
							textobj->SetFont("Default", *font);
						}
					}
				}

				TextStyle style;
				style.char_size = props["char_size"].get<unsigned>();
				style.char_spacing = props["char_spacing"].get<float>();
				style.line_spacing = props["line_spacing"].get<float>();
				style.fill_col = sf::Color(props["fill_col"].get<int>());
				style.outline_col = sf::Color(props["outline_col"].get<int>());
				style.style = static_cast<sf::Text::Style>(props["style"].get<uint32_t>());
				style.outline_thickness = props["outline_thickness"].get<float>();
				textobj->SetStyle(style);
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
		catch(json::type_error& e)
		{
			LOGERRORF("JSON type error: {}", e.what());
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

Scene LoadScene(const std::filesystem::path& path, const std::filesystem::path& asset_folder)
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
	scene.path = path;
	scene.name = path.filename().string();

	if (data.contains("objects"))
	{
		auto objects = data["objects"];
		LoadSceneObjects<Circle>(objects["circles"], scene, asset_folder);
		LoadSceneObjects<Sprite>(objects["sprites"], scene, asset_folder);
		LoadSceneObjects<Audio>(objects["audios"], scene, asset_folder);
		LoadSceneObjects<Text>(objects["texts"], scene, asset_folder);
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
	json audios;
	json texts;

	for (const auto& [name, object] : scene.objects_map)
	{
		Circle* circle = dynamic_cast<Circle*>(object.get());
		Sprite* sprite = dynamic_cast<Sprite*>(object.get());
		Audio* audio = dynamic_cast<Audio*>(object.get());
		Text* text = dynamic_cast<Text*>(object.get());

		if (circle != nullptr)
		{
			circles[name] = circle->Serialize();
		}
		else if (sprite != nullptr)
		{
			sprites[name] = sprite->Serialize();
		}
		else if (audio != nullptr)
		{
			audios[name] = audio->Serialize();
		}
		else if (text != nullptr)
		{
			texts[name] = text->Serialize();
		}		
	}

	objects["circles"] = circles;
	objects["sprites"] = sprites;
	objects["audios"] = audios;
	objects["texts"] = texts;
	//objects["..."] = ...

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
