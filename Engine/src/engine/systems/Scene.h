#pragma once

#include <EngineCore.h>
#include "nlohmann/json.hpp"
#include "engine/object_default/Object.h"

namespace Toad
{

using json = nlohmann::json;

struct ENGINE_API Scene
{
	Scene() : name("unnamed_scene") {}

	// scene name
	std::string name;

	// scene path
	std::filesystem::path path;

	// holds object instances in the scene
	std::vector<std::shared_ptr<Object>> objects_all;

	// objects that should get removed 
	std::vector<std::string> remove_objects;

	// objects that should be added 
	std::vector<std::shared_ptr<Object>> add_objects;

	///
	/// Is called once when switching to or starting this scene.
	///
	void Start();

	///
	/// Calls all Update functions in correct order on all object instances in this scene.
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
	std::shared_ptr<T> AddToScene(T&& object, bool is_begin_play = true)
	{
		static_assert(std::is_base_of_v<Object, T>, "Trying to add object of scene that doesn't inherit from Toad::Object class");

		std::string obj_name = object.name;
		bool found = false;
		bool found_in_queue = false;
		uint32_t count = 0;

		const auto check_name = [&](const std::vector<std::shared_ptr<Object>>& objects, bool& flag)
			{
				for (auto& obj : objects)
				{
					if (obj->name == obj_name)
					{
						count++;
						flag = true;
					}
				}
			};

		const auto adjust_name = [&](const std::vector<std::shared_ptr<Object>>& objects)
			{
				obj_name += " (" + std::to_string(count) + ')';
				auto it = std::ranges::find_if(objects, [&obj_name](const std::shared_ptr<Toad::Object>& obj) { return obj->name == obj_name; });
				while (it != objects.end())
				{
					obj_name = object.name + " (" + std::to_string(++count) + ')';
					it = std::ranges::find_if(objects, [&obj_name](const std::shared_ptr<Toad::Object>& obj) { return obj->name == obj_name; });
				}
			};

		check_name(objects_all, found);
		check_name(add_objects, found_in_queue);
	
		if (found)
			adjust_name(objects_all);

		else if (found_in_queue)
			adjust_name(add_objects);

		object.name = obj_name;

		if (is_begin_play)
		{
			add_objects.emplace_back(std::make_shared<T>(object));
			add_objects.back()->OnCreate();
			return std::dynamic_pointer_cast<T>(add_objects.back());
		}
		else
		{
			objects_all.emplace_back(std::make_shared<T>(object));
			objects_all.back()->OnCreate();
			return std::dynamic_pointer_cast<T>(objects_all.back());
		}
		
	}

	///
	/// Removes an object from the scene with the given name.
	///
	void RemoveFromScene(std::string_view obj_name, bool is_begin_play = true);

	///
	/// @returns 
	/// A pointer to object if found nullptr if no objects were found.
	///
	std::shared_ptr<Object> GetSceneObject(std::string_view obj_name);

	// for serializing a selection of objects 
	json Serialize(std::vector<Object*> v = {});
};

// to make sure scripts are added and loaded to objects make sure to update script registry before calling this function
// supply asset_folder only when in editor so it nows where to search for textures in the project 
ENGINE_API Scene LoadScene(const std::filesystem::path& path, const std::filesystem::path& asset_folder = {});

// for path specify only the folder to save to 
ENGINE_API void SaveScene(Scene& scene, const std::filesystem::path& path);

ENGINE_API void LoadSceneObjects(json objects, Scene& scene, const std::filesystem::path& asset_folder = {}, bool delete_old_objects = true);

}
