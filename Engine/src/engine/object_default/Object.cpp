#include "pch.h"
#include "Object.h"

#include "Engine/Engine.h"

namespace Toad
{
	Object::Object(): name("Object"), m_currentScene(Engine::Get().GetScene())
	{}

	Object::Object(std::string_view name): name(name), m_currentScene(Engine::Get().GetScene())
	{}

	Object::~Object() = default;

	void Object::Start()
	{}

	void Object::Render(sf::RenderWindow& window)
	{}

	void Object::Render(sf::RenderTexture& texture)
	{}

	void Object::Update()
	{}

	const Vec2f& Object::GetPosition()
	{
		return { -1, -1 };
	}

	void Object::SetPosition(const Vec2f& position)
	{
		const Vec2f& current_position = GetPosition();

		for (const std::string& child : m_children)
		{
			auto& obj = m_currentScene.objects_map[child];
			obj->SetPosition(obj->GetPosition() + (position - current_position));
		}
	}

	const std::set<std::string>& Object::GetChildren()
	{
		return m_children;
	}

	bool Object::AddChild(Object* object)
	{
		assert(object->GetParent() == this->name && "make sure this is already a parent of object");

		if (!m_children.contains(object->name))
		{
			m_children.insert(object->name);
			return true;
		}

		return false;
	}

	bool Object::RemoveChild(std::string_view obj_name)
	{
		if (m_children.contains(obj_name.data()))
		{
			m_children.erase(obj_name.data());
			return true;
		}

		return false;
	}

	void Object::SetParent(Object* object)
	{
		if (!m_parent.empty())
		{
			m_currentScene.objects_map[m_parent]->RemoveChild(name);
		}

		m_parent = object->name;
		object->AddChild(this);
	}

	const std::string& Object::GetParent()
	{
		return m_parent;
	}

	std::shared_ptr<Script> Object::GetScript(std::string_view name)
	{
		auto it = m_attachedScripts.find(name.data());
		if (it != m_attachedScripts.end())
		{
			return it->second;
		}
		return nullptr;
	}

	bool Object::RemoveScript(std::string_view script_name)
	{
		auto it = m_attachedScripts.find(script_name.data());
		if (it != m_attachedScripts.end())
		{
			m_attachedScripts.erase(it);
			return true;
		}

		return false;
	}

	void Object::AddScript(std::shared_ptr<Script> script)
	{
		m_attachedScripts[script->GetName()] = script;
	}

	const std::unordered_map<std::string, std::shared_ptr<Script>>& Object::GetAttachedScripts() const
	{
		return m_attachedScripts;
	}
}