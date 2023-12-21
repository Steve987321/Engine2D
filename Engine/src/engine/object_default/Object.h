#pragma once

#include <EngineCore.h>

#include "nlohmann/json.hpp"
#include "engine/default_scripts/Script.h"

namespace Toad
{
	class Script;

	using json = nlohmann::json;

///
/// An Object is the base class for different type of game objects
///	And holds scripts.
///
class ENGINE_API Object
{
public:
	explicit Object(std::string_view name) : name(name) {}
	Object() : name("Object") {}
	
	virtual ~Object() {}

	std::string name;

	virtual void Start()
	{}

	virtual void Render(sf::RenderWindow& window)
	{}

	// For imgui, only when using the editor
	virtual void Render(sf::RenderTexture& texture)
	{}

	virtual void Update()
	{}

	template <class T>
	T* GetScript()
	{
		for (const auto& script : m_attached_scripts | std::views::values)
		{
			auto res = dynamic_cast<T*>(script);
			if (res != nullptr)
				return res;
		}

		return nullptr;
	}

	std::shared_ptr<Script> GetScript(std::string_view name)
	{
		auto it = m_attached_scripts.find(name.data());
		if (it != m_attached_scripts.end())
		{
			return it->second;
		}
		return nullptr;
	}

	// faster than removing by script type
	bool RemoveScript(std::string_view script_name)
	{
		auto it = m_attached_scripts.find(script_name.data());
		if (it != m_attached_scripts.end())
		{
			m_attached_scripts.erase(it);
			return true;
		}

		return false;
	}

	// slower than removing by script string name
	template <class T>
	bool RemoveScript()
	{
		for (const auto& script : m_attached_scripts | std::views::values)
		{
			auto res = dynamic_cast<T*>(script);
			if (res != nullptr)
			{
				m_attached_scripts.erase(script->GetName());
				return true;
			}
		}

		return false;
	}

	void AddScript(std::shared_ptr<Script> script)
	{
		m_attached_scripts[script->GetName()] = script;
	}

	const std::unordered_map<std::string, std::shared_ptr<Script> >& GetAttachedScripts() const
	{
		return m_attached_scripts;
	}

protected:
	// attached scripts 
	std::unordered_map<std::string, std::shared_ptr<Script> > m_attached_scripts {};
};

}
