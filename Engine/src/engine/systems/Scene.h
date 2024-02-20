#pragma once

#include <EngineCore.h>
#include "nlohmann/json.hpp"
#include "engine/object_default/Object.h"

namespace Toad
{

using json = nlohmann::json;

#define GET_JSON_ELEMENT(val, data, key) assert(data.contains(key)); val = data[key]; 

struct ENGINE_API Scene
{
	Scene() : name("unnamed_scene") {}

	// scene name
	std::string name;

	// scene path
	std::filesystem::path path;

	// holds object instances in the scene
	std::vector<std::shared_ptr<Object>> objects;

	///
	/// Is called once when switching to or starting this scene.
	///
	void Start();

	///
	/// Calls Update on all object instances in this scene.
	///
	void Update();

	///
	///	Calls Render on all object instances in this scene.
	///
	void Render(sf::RenderWindow& window);
	void Render(sf::RenderTexture& texture);

	///
	/// @returns
	///	A pointer to the newly added object.
	///	Also checks if name of object is already here.
	///
	template <class T>
	T* AddToScene(T&& object)
	{
		static_assert(std::is_base_of_v<Object, T>, "Trying to add object of scene that doesn't inherit from Toad::Object class");

		std::string obj_name = object.name;
		bool found = false;
		uint32_t count = 0;
		for (auto& obj : objects)
		{
			if (obj->name == obj_name)
			{
				count++;
				found = true;
			}
		}
		if (found)
		{
			obj_name += " (" + std::to_string(count) + ')';
			auto it = std::ranges::find_if(objects, [&obj_name](const std::shared_ptr<Toad::Object>& obj) { return obj->name == obj_name; });
			while (it != objects.end())
			{
				obj_name = object.name + " (" + std::to_string(++count) + ')';
				it = std::ranges::find_if(objects, [&obj_name](const std::shared_ptr<Toad::Object>& obj) { return obj->name == obj_name; });
			}
		}

		object.name = obj_name;
		objects.emplace_back(std::make_shared<T>(object));
		objects.back()->OnCreate();
		return dynamic_cast<T*>(objects.back().get());
	}

	///
	/// @returns
	///	Whether an object has been removed with the given name.
	///
	bool RemoveFromScene(std::string_view obj_name);

	///
	/// @returns 
	/// A pointer to object if found nullptr if no objects were found.
	///
	Object* GetSceneObject(std::string_view obj_name);

	json Serialize();

	// for serializing a selection of objects 
	json Serialize(const std::vector<std::string>& objects);
};

// to make sure scripts are added and loaded to objects make sure to update script registry before calling this function
// supply asset_folder only when in editor so it nows where to search for textures in the project 
ENGINE_API Scene LoadScene(const std::filesystem::path& path, const std::filesystem::path& asset_folder = {});

// for path specify only the folder to save to 
ENGINE_API void SaveScene(Scene& scene, const std::filesystem::path& path);

ENGINE_API void LoadSceneObjects(json objects, Scene& scene, const std::filesystem::path& asset_folder = {});

}
