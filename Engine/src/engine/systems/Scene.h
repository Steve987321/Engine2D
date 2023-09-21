#pragma once

#include <EngineCore.h>

namespace Toad
{

class Object;
class Sprite;
class Circle;

struct ENGINE_API Scene
{
	Scene() = default;

	const char* name;

	// objects in scene
	//std::vector<Object> objects;
	std::unordered_map < std::string, std::shared_ptr<Object> > objects_map;

	///
	/// Is called when switching to this scene.
	///
	void Start();

	///
	/// Update the object instances in this scene.
	///
	void Update();

	///
	///	Draws the drawable objects to the window or texture
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
	///	whether an object has been removed with the given name
	///
	bool RemoveFromScene(std::string_view obj_name);

	///
	/// @returns 
	/// A pointer to object if found nullptr if no objects were found
	///
	Object* GetSceneObject(std::string_view obj_name);
};

}
