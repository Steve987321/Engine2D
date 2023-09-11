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
	std::unordered_map < std::string, std::shared_ptr<Object> > objectsMap;

	///
	/// Is called when switching to this scene.
	///
	void Start();

	///
	/// Update the object instances in this scene.
	///	Draws the drawable objects to the window and calls update on the scripts.
	///
	void Update(sf::RenderWindow& window);
	void Update(sf::RenderTexture& window);

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
		if (objectsMap.contains(objName))
		{
			objName += " (" + std::to_string(objectsMap.count(objName)) + ')';
			while (objectsMap.contains(objName))
			{
				objName += " (" + std::to_string(objectsMap.count(objName)) + ')';
			}
		}

		object.name = objName;
		objectsMap.insert({ objName, std::make_shared<T>(object) });
		return objectsMap[objName].get();

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
