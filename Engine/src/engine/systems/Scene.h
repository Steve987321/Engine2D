#pragma once

#include <EngineCore.h>
#include <filesystem>

namespace Toad
{

class Object;
class Sprite;
class Circle;

struct ENGINE_API Scene
{
	Scene() : name("unnamed_scene") {}

	// scene name
	const char* name;

	// holds object instances in the scene
	std::unordered_map < std::string, std::shared_ptr<Object> > objects_map;

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
	Object* GetSceneObject(std::string_view obj_name);
};

ENGINE_API Scene LoadScene(std::string_view path);
ENGINE_API Scene LoadScene(const std::filesystem::path& path);

// for path don't specify file name only the folder to save to 
// TODO: handle scene indexing 
ENGINE_API void SaveScene(const Scene& scene, std::string_view path);

}
