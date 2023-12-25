#include "pch.h"
#include "Object.h"

namespace Toad
{
	Object::Object(): name("Object")
	{}

	Object::Object(std::string_view name): name(name)
	{}

	Object::~Object()
	{}

	void Object::Start()
	{}

	void Object::Render(sf::RenderWindow& window)
	{}

	void Object::Render(sf::RenderTexture& texture)
	{}

	void Object::Update()
	{}

	std::shared_ptr<Script> Object::GetScript(std::string_view name)
	{
		auto it = m_attached_scripts.find(name.data());
		if (it != m_attached_scripts.end())
		{
			return it->second;
		}
		return nullptr;
	}

	bool Object::RemoveScript(std::string_view script_name)
	{
		auto it = m_attached_scripts.find(script_name.data());
		if (it != m_attached_scripts.end())
		{
			m_attached_scripts.erase(it);
			return true;
		}

		return false;
	}

	void Object::AddScript(std::shared_ptr<Script> script)
	{
		m_attached_scripts[script->GetName()] = script;
	}

	const std::unordered_map<std::string, std::shared_ptr<Script>>& Object::GetAttachedScripts() const
	{
		return m_attached_scripts;
	}
}
