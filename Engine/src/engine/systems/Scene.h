#pragma once

#include <EngineCore.h>
#include "nlohmann/json.hpp"
#include "engine/default_objects/Object.h"
#include "engine/PlaySession.h"

namespace Toad
{

using json = nlohmann::ordered_json;

class ENGINE_API Scene
{
public:
	Scene()
		: name("unnamed_scene")
	{}

	struct TObjectToBeAdded
	{
		std::shared_ptr<Object> object;
		int32_t index;
	};

	static std::vector<Scene> scenes;

	// active scene
	static Scene& current_scene;

	static void SetScene(Scene* scene);

	// scene name
	std::string name;

	// scene path
	std::filesystem::path path;

	// holds object instances in the scene
	std::vector<std::shared_ptr<Object>> objects_all;

	// objects that should get removed 
	std::vector<std::string> remove_objects;

	// objects that should be added 
	std::vector<TObjectToBeAdded> add_objects;

	// set when something has been removed from the scene, needs to be manually set back to its default value
	bool removed_from_scene = false;

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
	void Render(sf::RenderTarget& target);

	///
	/// Called when scene is ended, Scene parameter will be nullptr if the application is stopped, else it will switch to that scene.
	///
	void End(Scene* next_scene);

	///
	/// @returns
	///	A pointer to the newly added object.
	///	Also checks if name of object is already here.
	///
	template <class T>
	std::shared_ptr<T> AddToScene(T&& object, int32_t index = -1, bool insert = false)
	{
		static_assert(std::is_base_of_v<Object, T>, "Trying to add object of scene that doesn't inherit from Toad::Object class");

		std::string obj_name = std::move(object.name);
		bool found = false;
		bool found_in_queue = false;
		uint32_t count = 0;

		const auto check_name_queue = [&](const std::vector<TObjectToBeAdded>& objects, bool& flag)
			{
				for (const auto& obj : objects)
				{
					if (obj.object->name == obj_name)
					{
						count++;
						flag = true;
					}
				}
			};

		const auto check_name = [&](const std::vector<std::shared_ptr<Object>>& objects, bool& flag)
			{
				for (const auto& obj : objects)
				{
					if (obj && obj->name == obj_name)
					{
						count++;
						flag = true;
					}
				}
			};

		const auto adjust_name = [&](const std::vector<std::shared_ptr<Object>>& objects)
			{
				obj_name += " (" + std::to_string(count) + ')';
				auto it = std::find_if(objects.begin(), objects.end(), [&obj_name](const std::shared_ptr<Toad::Object>& obj) { return obj->name == obj_name; });
				while (it != objects.end())
				{
					obj_name = object.name + " (" + std::to_string(++count) + ')';
					it = std::find_if(objects.begin(), objects.end(), [&obj_name](const std::shared_ptr<Toad::Object>& obj) { return obj->name == obj_name; });
				}
			};

		const auto adjust_name_queue = [&](const std::vector<TObjectToBeAdded>& objects)
			{
				obj_name += " (" + std::to_string(count) + ')';
				auto it = std::find_if(
					objects.begin(),
					objects.end(),
					[&obj_name](const TObjectToBeAdded& obj)
					{ 
						return obj.object->name == obj_name;
					});
				while (it != objects.end())
				{
					obj_name = object.name + " (" + std::to_string(++count) + ')';
					it = std::find_if(
						objects.begin(), 
						objects.end(), 
						[&obj_name](const TObjectToBeAdded& obj)
						{ 
							return obj.object->name == obj_name;
						});
				}
			};

		check_name(objects_all, found);
		check_name_queue(add_objects, found_in_queue);
	
		if (found)
			adjust_name(objects_all);

		else if (found_in_queue)
			adjust_name_queue(add_objects);

		object.name = obj_name;

		if (IsBeginPlay())
		{
			// add to a queue 
			add_objects.emplace_back(std::make_shared<T>(object), index);
			add_objects.back().object->OnCreate();
			return std::dynamic_pointer_cast<T>(add_objects.back().object);
		}
		else
		{
			if (index < 0)
			{
				objects_all.emplace_back(std::make_shared<T>(object));
				objects_all.back()->OnCreate();
				return std::dynamic_pointer_cast<T>(objects_all.back());
			}
			else 
			{
				if ((size_t)index + 1 > objects_all.size())
					objects_all.resize((size_t)index + 1);
				
				if (insert)
				{
					objects_all.insert(objects_all.begin() + index, std::make_shared<T>(object));
					objects_all[index]->OnCreate();
				}
				else
				{
					objects_all[index] = std::make_shared<T>(object);
					objects_all[index]->OnCreate();
				}
				
				return std::dynamic_pointer_cast<T>(objects_all[index]);
			}
		}
		
	}

	///
	/// Removes an object from the scene with the given name.
	///
	void RemoveFromScene(std::string_view obj_name);

	///
	/// @returns 
	/// A pointer to object if found nullptr if no objects were found.
	///
	std::shared_ptr<Object> GetSceneObject(std::string_view obj_name);

	// for serializing a selection of objects 
	json Serialize(std::vector<Object*> v = {});

private:
	void RegisterScriptFunctions();

	std::vector<std::function<void(Object&)>> start_methods;
	std::vector<std::function<void(Object&)>> update_methods;
	std::vector<std::function<void(Object&)>> fixed_update_methods;
};

// make sure scripts are added and loaded to objects 
// make sure to update script registry before calling this function
// supply asset_folder only when in editor so it knows where to search for textures in the project 
ENGINE_API Scene& LoadScene(const std::filesystem::path& path, const std::filesystem::path& asset_folder = {});

// for path specify only the folder to save to 
ENGINE_API void SaveScene(Scene& scene, const std::filesystem::path& path);

ENGINE_API void LoadSceneObjects(json objects, Scene& scene, const std::filesystem::path& asset_folder = {}, bool delete_old_objects = true, bool insert = false);

extern Scene empty_scene;

}
