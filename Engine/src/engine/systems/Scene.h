#pragma once

namespace Toad
{
	class Object;

struct Scene
{
	const char* name;

	// objects in scene
	std::vector<Object> objects;

	void Start();
	void Update();
};

}
