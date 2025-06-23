#include "pch.h"
#include "Scene.h"

#include "engine/default_objects/Object.h"
#include "engine/default_objects/Sprite.h"
#include "engine/default_objects/Circle.h"

#include "engine/PlaySession.h"
#include "engine/Engine.h"
#include "engine/systems/Time.h"
#include "nlohmann/json.hpp"

#include "engine/utils/Helpers.h"

namespace Toad
{

using json = nlohmann::ordered_json;
Scene empty_scene;
Scene& Scene::current_scene = empty_scene;
std::vector<Scene> Scene::scenes;

void Scene::SetScene(Scene* scene)
{
	current_scene.End(scene);

	if (!scene)
		return;

	current_scene = *scene;

	if (begin_play)
		current_scene.Start();
}

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
	
	static float fdt = Time::GetFixedDeltaTime();
	static float time_acc = fdt;
	time_acc += Time::GetDeltaTime();
	while (time_acc >= fdt)
	{
		for (auto& obj : objects_all)
		{
			obj->FixedUpdate();
		}

		time_acc -= fdt;
	}

	for (auto& obj : objects_all)
	{
		obj->LateUpdate();
	}

	for (const auto& obj_name : remove_objects)
	{
		auto it = std::remove_if(
			objects_all.begin(),
			objects_all.end(),
			[&obj_name](const std::shared_ptr<Object>& obj) 
			{
				return obj->name == obj_name;
			});

		objects_all.erase(it, objects_all.end());
	}

	for (const auto& obj : add_objects)
	{
		if (obj.index == -1)
		{
			objects_all.emplace_back(obj.object);
		}
		else
		{
			if ((size_t)obj.index + 1 > objects_all.size())
				objects_all.resize((size_t)obj.index + 1);

			objects_all[obj.index] = obj.object;
		}
	}

	remove_objects.clear();
	add_objects.clear();
}

void Scene::Render(sf::RenderTarget& target)
{
	for (auto& obj : objects_all)
	{
		obj->Render(target);
	}
}

void Scene::End(Scene* next_scene)
{
	for (auto& obj : objects_all)
	{
		obj->End(next_scene);
	}
}

void Scene::RemoveFromScene(std::string_view obj_name, bool is_begin_play)
{
	if (is_begin_play)
		remove_objects.emplace_back(obj_name);
	else
	{
		auto it = std::remove_if(
			objects_all.begin(),
			objects_all.end(),
			[&obj_name](const std::shared_ptr<Object>& obj)
			{
				return obj->name == obj_name;
			});

		objects_all.erase(it, objects_all.end());
	}

	removed_from_scene = true;
}

std::shared_ptr<Object> Scene::GetSceneObject(std::string_view obj_name) 
{
	for (auto& obj : objects_all)
	{
		if (obj && obj->name == obj_name)
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
	json empties;
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

	int32_t index = 0;

	for (const auto& object : v)
	{
		Circle* circle = dynamic_cast<Circle*>(object);
		Sprite* sprite = dynamic_cast<Sprite*>(object);
		Audio* audio = dynamic_cast<Audio*>(object);
		Text* text = dynamic_cast<Text*>(object);
		Camera* cam = dynamic_cast<Camera*>(object);

		if (circle)
		{
			circles[object->name] = circle->Serialize();
			circles[object->name]["index"] = index;
		}
		else if (sprite)
		{
			sprites[object->name] = sprite->Serialize();
			sprites[object->name]["index"] = index;
		}
		else if (audio)
		{
			audios[object->name] = audio->Serialize();
			audios[object->name]["index"] = index;
		}
		else if (text)
		{
			texts[object->name] = text->Serialize();
			texts[object->name]["index"] = index;
		}
		else if (cam)
		{
			cameras[object->name] = cam->Serialize();
			cameras[object->name]["index"] = index;
		}
		else
		{
			empties[object->name] = object->Serialize();
			empties[object->name]["index"] = index;
		}

		index++;
	}

	objects["empty"] = empties;
	objects["circles"] = circles;
	objects["sprites"] = sprites;
	objects["audios"] = audios;
	objects["texts"] = texts;
	objects["cameras"] = cameras;
	//objects["..."] = ...

	data["objects"] = objects;

	return data;
}

void Scene::RegisterScriptFunctions()
{
	using Fp = void(Script::*)(Object*);

	for (const auto& obj : objects_all)
	{
		for (const auto& [name, script] : obj->GetAttachedScripts())
		{

		}
	}
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
		{obj["left"].get<int>(), obj["top"].get<int>()},
		{obj["width"].get<int>(), obj["height"].get<int>()}
	};
}

Scene& LoadScene(const std::filesystem::path& path, const std::filesystem::path& asset_folder)
{
	std::ifstream in(path);
	Scene scene;
	scene.name = "invalid";
	Scene::scenes.emplace_back(scene);

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
			return Scene::scenes.back();
		}
	}

	scene.path = path;
	scene.name = path.filename().string();

	LoadSceneObjects(data, scene, asset_folder);

	if (Scene::scenes.back().name == "invalid")
		Scene::scenes.back() = std::move(scene);
	else
		Scene::scenes.emplace_back(std::move(scene));

	return Scene::scenes.back();
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

	static_assert(std::is_base_of_v<Object, T>, "Can't load object into the scene that isn't inherited from Toad::Object");

	std::queue<std::pair<std::string, std::string>> set_object_parents_queue{};

	for (const auto& object : objects.items())
	{
		try
		{
			json props;
			int32_t index = -1;
			float x = 0.f;
			float y = 0.f;
            
            LOGDEBUGF("{}", object.value());
            
			GET_JSON_ELEMENT(props, object.value(), "properties");
			GET_JSON_ELEMENT(index, object.value(), "index");
			GET_JSON_ELEMENT(x, props, "posx");
			GET_JSON_ELEMENT(y, props, "posy");
            
			Object* newobj = scene.AddToScene(T(object.key()), Toad::begin_play, index).get();
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
					newobj->SetParent(parent_obj);
				else
					set_object_parents_queue.emplace(newobj->name, parent_name);
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
					std::string rel_path_str;
					json rect; 

					GET_JSON_ELEMENT(rel_path_str, props, "texture_loc");
					GET_JSON_ELEMENT(rect, props, "texture_rect");

					sf::Texture* new_tex = ResourceManager::GetTextures().Get(rel_path_str);
					sf::IntRect tex_rect = GetRectFromJSON(rect);

					if (new_tex == nullptr)
					{
#ifdef TOAD_EDITOR
						sf::Texture tex = GetTexFromPath(asset_folder / std::filesystem::path(rel_path_str));
#else
						sf::Texture tex = GetTexFromPath(std::filesystem::path(rel_path_str));
#endif
						new_tex = ResourceManager::GetTextures().Add(rel_path_str, tex);
					}
                    
					circleobj->SetTexture(rel_path_str, new_tex);
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

				GET_JSON_ELEMENT(u32fill_col, props, "fill_col");
				GET_JSON_ELEMENT(scale.x, props, "scalex");
				GET_JSON_ELEMENT(scale.y, props, "scaley"); 
				GET_JSON_ELEMENT(origin.x, props, "originx");
				GET_JSON_ELEMENT(origin.y, props, "originy");
				GET_JSON_ELEMENT(rotation, props, "rotation");

				fill_col = sf::Color(u32fill_col);

                std::string path_str;
                json rect;

                GET_JSON_ELEMENT(path_str, props, "texture_loc");
                GET_JSON_ELEMENT(rect, props, "texture_rect");

                sf::Texture* new_tex = ResourceManager::GetTextures().Get(path_str);
                sf::IntRect tex_rect = GetRectFromJSON(rect);

                if (new_tex == nullptr)
                {
#ifdef TOAD_EDITOR
                    sf::Texture tex = GetTexFromPath(asset_folder / std::filesystem::path(path_str));
#else
                    sf::Texture tex = GetTexFromPath(std::filesystem::path(path_str));
#endif
                    new_tex = ResourceManager::GetTextures().Add(path_str, tex);
                }
                
                spriteobj->SetTexture(path_str, new_tex);
                sprite.setTextureRect(tex_rect);

				sprite.setColor(fill_col);
				sprite.setRotation(sf::degrees(rotation));
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

					AudioSource* managed_audio_source = ResourceManager::GetAudioSources().Add(rel_path, new_audio_source);
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
					sf::Font* font = ResourceManager::GetFonts().Get("Default");
					if (font != nullptr)
					{
						textobj->SetFont("Default", *font);
					}
					else
					{
						sf::Font arial;

						if (!arial.openFromFile("C:\\Windows\\Fonts\\Arial.ttf"))
						{
							LOGWARN("[Scene] Can't find C:\\Windows\\Fonts\\Arial.ttf");
						}
						else
						{
							font = ResourceManager::GetFonts().Add("Default", arial);
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
				auto gscripts = Toad::GetGameScriptsRegister();
				if (gscripts.empty())
				{
					LOGWARN("[Scene] Scripts register is empty");
				}
				if (auto it = gscripts.find(script.key()); it != gscripts.end())
				{
					if (!it->second)
						continue;

					newobj->AddScript(it->second->Clone());
					newobj->GetScript(script.key())->ExposeVars();
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

ENGINE_API void LoadSceneObjects(json objects, Scene& scene, const std::filesystem::path& asset_folder, bool delete_old_objects)
{
	if (objects.contains("objects"))
	{
		bool restart = false;
		if (Toad::begin_play)
		{
			restart = true;
			Toad::StopGameSession();
		}

		if (delete_old_objects)
			for (const auto& obj : Scene::current_scene.objects_all)
				if (obj)
					obj->Destroy();
			
		auto& objectsall = objects["objects"];
		LoadSceneObjectsOfType<Object>(objectsall["empty"], scene, asset_folder);
		LoadSceneObjectsOfType<Circle>(objectsall["circles"], scene, asset_folder);
		LoadSceneObjectsOfType<Sprite>(objectsall["sprites"], scene, asset_folder);
		LoadSceneObjectsOfType<Audio>(objectsall["audios"], scene, asset_folder);
		LoadSceneObjectsOfType<Text>(objectsall["texts"], scene, asset_folder);
		LoadSceneObjectsOfType<Camera>(objectsall["cameras"], scene, asset_folder);

		if (restart) 
		{
			Toad::StartGameSession();
		}
	}
}

}
