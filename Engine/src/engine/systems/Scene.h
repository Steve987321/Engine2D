#pragma once

#include <EngineCore.h>
#include "nlohmann/json.hpp"

namespace Toad
{

class Object;

using json = nlohmann::json;

struct ENGINE_API Scene
{
	Scene() : name("unnamed_scene") {}

	// scene name
	std::string name;

	// scene path
	std::filesystem::path path;

	// holds object instances in the scene
	std::unordered_map < std::string, std::shared_ptr<Object> > objects_map;

	///
	/// Is called once when switching to or starting this scene.
	///
	void Start();

	///
	/// Calls FixedUpdate on all object instances in this scene.
	///
	void FixedUpdate();

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
	Object* AddToScene(T&& object)
	{
		static_assert(std::is_base_of_v<Object, T>, "Trying to add object of scene that doesn't inherit from Toad::Object class");

		std::string objName = object.name;
		if (objects_map.contains(objName))
		{
			auto count = objects_map.count(objName);
			objName += " (" + std::to_string(count) + ')';
			while (objects_map.contains(objName))
			{
				objName = object.name + " (" + std::to_string(++count) + ')';
			}
		}

		object.name = objName;
		objects_map.insert({ objName, std::make_shared<T>(object) });
		return objects_map[objName].get();
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
	Object* GetSceneObject(std::string_view obj_name) const;

	json Serialize() const;

	// for serializing a selection of objects 
	json Serialize(const std::vector<std::string>& objects) const;
};

// to make sure scripts are added and loaded to objects make sure to update script registry before calling this function
// supply asset_folder only when in editor so it nows where to search for textures in the project 
ENGINE_API Scene LoadScene(const std::filesystem::path& path, const std::filesystem::path& asset_folder = {});

// for path specify only the folder to save to 
ENGINE_API void SaveScene(const Scene& scene, const std::filesystem::path& path);

ENGINE_API void LoadSceneObjects(json objects, Scene& scene, const std::filesystem::path& asset_folder = {});

}
