#pragma once

#include <EngineCore.h>

namespace Toad
{

class Object;

struct ENGINE_API Scene
{
	Scene() = default;

	const char* name;

	// objects in scene
	//std::vector<Object> objects;
	std::unordered_map < std::string, Object > objectsMap;

	void Start();
	void Update(sf::RenderWindow& window);

	///
	/// @returns
	///	A pointer to the newly added object
	///
	///	@note
	///	The old object shouldn't be used anymore
	///
	Object* AddToScene(const Object& obj);

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
