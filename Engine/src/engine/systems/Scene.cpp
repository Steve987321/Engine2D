#include "pch.h"
#include "Scene.h"

#include "engine/object_default/Object.h"
#include "engine/object_default/Sprite.h"
#include "engine/object_default/Circle.h"

#include "Engine/Engine.h"

#include "nlohmann/json.hpp"

#include "engine/Helpers.h"

namespace Toad
{

using json = nlohmann::json;

void Scene::Start()
{
	for (auto& obj : objects_all)
	{
		obj->Start();
	}
}

void Scene::Update()
{
	for (auto& obj : objects_all)
	{
		obj->Update();
	}
	
	static float time = 0.04f;
	time += Toad::Engine::Get().GetDeltaTime().asSeconds();
	while (time >= 0.04f) {

		for (auto& obj : objects_all)
		{
			obj->FixedUpdate();
		}

		time -= 0.04f;
	}

	for (auto& obj : objects_all)
	{
		obj->LateUpdate();
	}
}

void Scene::Render(sf::RenderTexture& texture)
{
	for (auto& obj : objects_all)
	{
		obj->Render(texture);
	}
}

void Scene::Render(sf::RenderWindow& window)
{
	for (auto& obj : objects_all)
	{
		obj->Render(window);
	}
}

bool Scene::RemoveFromScene(std::string_view obj_name)
{
	uint32_t n = objects_all.size();
	auto it = std::remove_if(objects_all.begin(), objects_all.end(), [&obj_name](const std::shared_ptr<Object>& obj) {
		return obj->name == obj_name;
	});

	objects_all.erase(it, objects_all.end());
	
	return n != objects_all.size();
}

std::shared_ptr<Object> Scene::GetSceneObject(std::string_view obj_name) 
{
	for (auto& obj : objects_all)
	{
		if (obj->name == obj_name)
		{
			return obj;
		}
	}
	return nullptr;
}

json Scene::Serialize(std::vector<Object*> v)
{
	json data; 

	// object types
	json objects; // all 
	json circles;
	json sprites;
	json audios;
	json texts;
	json cameras;

	if (v.empty())
	{
		for (const auto& obj : objects_all)
		{
			v.push_back(obj.get());
		}
	}
	for (const auto& object : v)
	{
		Circle* circle = dynamic_cast<Circle*>(object);
		Sprite* sprite = dynamic_cast<Sprite*>(object);
		Audio* audio = dynamic_cast<Audio*>(object);
		Text* text = dynamic_cast<Text*>(object);
		Camera* cam = dynamic_cast<Camera*>(object);

		if (circle != nullptr)
		{
			circles[object->name] = circle->Serialize();
		}
		else if (sprite != nullptr)
		{
			sprites[object->name] = sprite->Serialize();
		}
		else if (audio != nullptr)
		{
			audios[object->name] = audio->Serialize();
		}
		else if (text != nullptr)
		{
			texts[object->name] = text->Serialize();
		}
		else if (cam != nullptr)
		{
			cameras[object->name] = cam->Serialize();
		}
	}

	objects["circles"] = circles;
	objects["sprites"] = sprites;
	objects["audios"] = audios;
	objects["texts"] = texts;
	objects["cameras"] = cameras;
	//objects["..."] = ...

	data["objects"] = objects;

	return data;
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
	int left = 0;
	int top = 0;
	int width = 0;
	int height = 0;

	GET_JSON_ELEMENT(left, obj, "left");
	GET_JSON_ELEMENT(top, obj, "top");
	GET_JSON_ELEMENT(width, obj, "width");
	GET_JSON_ELEMENT(height, obj, "height");

	return sf::IntRect{
		obj["left"].get<int>(),
		obj["top"].get<int>(),
		obj["width"].get<int>(),
		obj["height"].get<int>(),
	};
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

	LoadSceneObjects(data, scene, asset_folder);

	return scene;
}

void SaveScene(Scene& scene, const std::filesystem::path& path)
{
	json data = scene.Serialize();

	std::string dir = path.string();

	if (dir.find(PATH_SEPARATOR) != std::string::npos && !dir.ends_with(PATH_SEPARATOR))
		dir += PATH_SEPARATOR;

	std::string full = dir + scene.name;
	if (!scene.name.ends_with(".TSCENE"))
	{
		full += ".TSCENE";
	}

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

template <typename T>
ENGINE_API inline void LoadSceneObjectsOfType(json objects, Scene& scene, const std::filesystem::path& asset_folder = {})
{
#ifdef TOAD_EDITOR
	assert(!asset_folder.empty() && "asset_folder argument should be used when in toad editor");
#endif

	std::queue<std::pair<std::string, std::string>> set_object_parents_queue{};

	for (const auto& object : objects.items())
	{
		try
		{
			json props;
			float x = 0.f;
			float y = 0.f;

			GET_JSON_ELEMENT(props, object.value(), "properties");
			GET_JSON_ELEMENT(x, props, "posx");
			GET_JSON_ELEMENT(y, props, "posy");

			Object* newobj = scene.AddToScene(T(object.key())).get();
			Sprite* spriteobj = dynamic_cast<Sprite*>(newobj);
			Circle* circleobj = dynamic_cast<Circle*>(newobj);
			Audio* audioobj = dynamic_cast<Audio*>(newobj);
			Text* textobj = dynamic_cast<Text*>(newobj);
			Camera* camobj = dynamic_cast<Camera*>(newobj);

			newobj->SetPosition({ x,y });

			std::string parent_name = props["parent"].get<std::string>();
			if (!parent_name.empty())
			{
				Object* parent_obj = scene.GetSceneObject(parent_name).get();
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
				sf::Color fill_col{};
				sf::Color outline_col{};
				uint32_t u32fill_col = 0;
				uint32_t u32outline_col = 0;
				float rotation = 0;
				sf::Vector2f scale = {};
				float radius = 0; 
				bool has_texture = false; 

				GET_JSON_ELEMENT(u32fill_col, props, "fill_col");
				GET_JSON_ELEMENT(u32outline_col, props, "outline_col");
				GET_JSON_ELEMENT(rotation, props, "rotation");
				GET_JSON_ELEMENT(scale.x, props, "scalex");
				GET_JSON_ELEMENT(scale.y, props, "scaley");
				GET_JSON_ELEMENT(radius, props, "radius");
				GET_JSON_ELEMENT(has_texture, props, "has_texture");

				fill_col = sf::Color(u32fill_col);
				outline_col = sf::Color(u32outline_col);

				if (has_texture)
				{
					std::string path_str;
					json rect; 

					GET_JSON_ELEMENT(path_str, props, "texture_loc");
					GET_JSON_ELEMENT(rect, props, "texture_rect");

					sf::Texture* new_tex = Engine::Get().GetResourceManager().GetTexture(path_str);
					sf::IntRect tex_rect = GetRectFromJSON(rect);

					if (new_tex == nullptr)
					{
#ifdef TOAD_EDITOR
						sf::Texture tex = GetTexFromPath(asset_folder / std::filesystem::path(path_str));
#else
						sf::Texture tex = GetTexFromPath(std::filesystem::path(path_str));
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
				sf::Color fill_col = {};
				sf::Vector2f scale = {};
				sf::Vector2f origin = {};
				uint32_t u32fill_col = 0;
				float rotation = 0;
				bool has_texture = false;

				GET_JSON_ELEMENT(u32fill_col, props, "fill_col");
				GET_JSON_ELEMENT(scale.x, props, "scalex");
				GET_JSON_ELEMENT(scale.y, props, "scaley"); 
				GET_JSON_ELEMENT(origin.x, props, "originx");
				GET_JSON_ELEMENT(origin.y, props, "originy");
				GET_JSON_ELEMENT(rotation, props, "rotation");
				GET_JSON_ELEMENT(has_texture, props, "has_texture");

				fill_col = sf::Color(u32fill_col);

				if (has_texture)
				{
					std::string path_str;
					json rect;

					GET_JSON_ELEMENT(path_str, props, "texture_loc");
					GET_JSON_ELEMENT(rect, props, "texture_rect");

					sf::Texture* new_tex = Engine::Get().GetResourceManager().GetTexture(path_str);
					sf::IntRect tex_rect = GetRectFromJSON(rect);

					if (new_tex == nullptr)
					{
#ifdef TOAD_EDITOR
						sf::Texture tex = GetTexFromPath(asset_folder / std::filesystem::path(path_str));
#else
						sf::Texture tex = GetTexFromPath(std::filesystem::path(path_str));
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
				sprite.setOrigin(origin);
			}
			else if (audioobj != nullptr)
			{
				float play_from_src = 0.f;
				float volume = 0.f;
				float pitch = 0.f;
				float spatial_x = 0.f;
				float spatial_y = 0.f;
				float spatial_z = 0.f;

				GET_JSON_ELEMENT(play_from_src, props, "play_from_source")
				GET_JSON_ELEMENT(volume, props, "volume")
				GET_JSON_ELEMENT(pitch, props, "pitch")
				GET_JSON_ELEMENT(spatial_x, props, "audio_posx")
				GET_JSON_ELEMENT(spatial_y, props, "audio_posy")
				GET_JSON_ELEMENT(spatial_z, props, "audio_posz")

				audioobj->ShouldPlayFromSource(play_from_src);
				audioobj->SetVolume(volume);
				audioobj->SetPitch(pitch);
				audioobj->SetAudioPosition({ spatial_x, spatial_y, spatial_z });

				if (props.contains("audio_source"))
				{
					json audio_source_data = props["audio_source"];

					std::string full_path;
					std::string rel_path;
					bool valid_buf = false;

					GET_JSON_ELEMENT(full_path, audio_source_data, "full_path");
					GET_JSON_ELEMENT(rel_path, audio_source_data, "rel_path");
					GET_JSON_ELEMENT(valid_buf, audio_source_data, "has_valid_buf");

					AudioSource new_audio_source;
					new_audio_source.has_valid_buffer = valid_buf;
					new_audio_source.full_path = full_path;
					new_audio_source.relative_path = rel_path;

					if (valid_buf)
					{
						sf::SoundBuffer sb;
#ifdef TOAD_EDITOR
						if (!sb.loadFromFile(full_path))
						{
							LOGERRORF("[Scene] Loading soundbuffer file from path {} failed", full_path);
						}
#else
						sb.loadFromFile(new_audio_source.relative_path.string());
#endif

						new_audio_source.sound_buffer = sf::SoundBuffer(sb);
					}

					AudioSource* managed_audio_source = Engine::Get().GetResourceManager().AddAudioSource(rel_path, new_audio_source);
					audioobj->SetSource(managed_audio_source);
				}
			}
			else if (textobj != nullptr)
			{
				std::string text; 
				std::string font_loc; 
				float rotation = 0;

				GET_JSON_ELEMENT(text, props, "text");
				GET_JSON_ELEMENT(font_loc, props, "font_loc");
				GET_JSON_ELEMENT(rotation, props, "rotation");

				textobj->SetText(text);
				textobj->SetRotation(rotation);

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
							LOGWARN("[Scene] Can't find C:\\Windows\\Fonts\\Arial.ttf");
						}
						else
						{
							font = Engine::Get().GetResourceManager().AddFont("Default", arial);
							textobj->SetFont("Default", *font);
						}
					}
				}

				TextStyle style;

				uint32_t u32style = 0;
				uint32_t u32fill_col = 0;
				uint32_t u32outline_col = 0;
				
				GET_JSON_ELEMENT(style.char_size, props, "char_size");
				GET_JSON_ELEMENT(style.char_spacing, props, "char_spacing");
				GET_JSON_ELEMENT(style.line_spacing, props, "line_spacing");
				GET_JSON_ELEMENT(u32fill_col, props, "fill_col");
				GET_JSON_ELEMENT(u32outline_col, props, "outline_col");
				GET_JSON_ELEMENT(style.outline_thickness, props, "outline_thickness");
				GET_JSON_ELEMENT(u32style, props, "style");

				style.fill_col = sf::Color(u32fill_col);
				style.outline_col = sf::Color(u32outline_col);
				style.style = static_cast<sf::Text::Style>(u32style);

				textobj->SetStyle(style);
			}
			else if (camobj != nullptr)
			{
				float rotation = 0;
				bool active = false;
				float sizex = 0;
				float sizey = 0;

				GET_JSON_ELEMENT(rotation, props, "rotation");
				GET_JSON_ELEMENT(active, props, "cam_active");
				GET_JSON_ELEMENT(sizex, props, "sizex");
				GET_JSON_ELEMENT(sizey, props, "sizey");

				camobj->SetRotation(rotation);
				camobj->SetSize({ sizex, sizey });

				if (active)
				{
					camobj->ActivateCamera();
				}
				else
				{
					camobj->DeactivateCamera();
				}
			}

			for (const auto& script : object.value()["scripts"].items())
			{
				auto gscripts = Engine::Get().GetGameScriptsRegister();
				if (gscripts.empty())
				{
					LOGWARN("[Scene] Scripts register is empty");
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
								if (vars.b[j.key()])
									*vars.b[j.key()] = j.value().get<bool>();
								else
								{
									LOGWARNF("[Scene] Script variable (bool) {} for script {} is null and is getting skipped & disabled, make sure it's exposed", j.key().c_str(), script.key().c_str());
								}
							}
							break;
						case (int)TypesMap::flt:
							for (const auto& j : script_vars.value().items())
							{
								if (vars.flt[j.key()])
									*vars.flt[j.key()] = j.value().get<float>();
								else
								{
									LOGWARNF("[Scene] Script variable (float) {} for script {} is null and is getting skipped & disabled, make sure it's exposed", j.key().c_str(), script.key().c_str());
								}
							}
							break;
						case (int)TypesMap::i8:
							for (const auto& j : script_vars.value().items())
							{
								if (vars.i8[j.key()])
									*vars.i8[j.key()] = j.value().get<int8_t>();
								else
								{
									LOGWARNF("[Scene] Script variable (int8) {} for script {} is null and is getting skipped & disabled, make sure it's exposed", j.key().c_str(), script.key().c_str());
								}
							}
							break;
						case (int)TypesMap::i16:
							for (const auto& j : script_vars.value().items())
							{
								if (vars.i16[j.key()])
									*vars.i16[j.key()] = j.value().get<int16_t>();
								else
								{
									LOGWARNF("[Scene] Script variable (int16) {} for script {} is null and is getting skipped & disabled, make sure it's exposed", j.key().c_str(), script.key().c_str());
								}
							}
							break;
						case (int)TypesMap::i32:
							for (const auto& j : script_vars.value().items())
							{
								if (vars.i32[j.key()])
									*vars.i32[j.key()] = j.value().get<int32_t>();
								else
								{
									LOGWARNF("[Scene] Script variable (int32) {} for script {} is null and is getting skipped & disabled, make sure it's exposed", j.key().c_str(), script.key().c_str());
								}
							}
							break;
						case (int)TypesMap::str:
							for (const auto& j : script_vars.value().items())
							{
								if (vars.str[j.key()])
									*vars.str[j.key()] = j.value().get<std::string>();
								else
								{
									LOGWARNF("[Scene] Script variable (string) {} for script {} is null and is getting skipped & disabled, make sure it's exposed", j.key().c_str(), script.key().c_str());
								}
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
		catch (json::type_error& e)
		{
			LOGERRORF("JSON type error: {}", e.what());
		}
	}

	while (!set_object_parents_queue.empty())
	{
		const auto& [child, parent] = set_object_parents_queue.front();
		Object* child_obj = scene.GetSceneObject(child).get();
		Object* parent_obj = scene.GetSceneObject(parent).get();

		child_obj->SetParent(parent_obj);

		set_object_parents_queue.pop();
	}
}


ENGINE_API void LoadSceneObjects(json objects, Scene& scene, const std::filesystem::path& asset_folder)
{
	if (objects.contains("objects"))
	{
		auto& objectsall = objects["objects"];
		LoadSceneObjectsOfType<Circle>(objectsall["circles"], scene, asset_folder);
		LoadSceneObjectsOfType<Sprite>(objectsall["sprites"], scene, asset_folder);
		LoadSceneObjectsOfType<Audio>(objectsall["audios"], scene, asset_folder);
		LoadSceneObjectsOfType<Text>(objectsall["texts"], scene, asset_folder);
		LoadSceneObjectsOfType<Camera>(objectsall["cameras"], scene, asset_folder);
	}
}

}
