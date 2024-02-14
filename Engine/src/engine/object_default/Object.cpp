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

	void Object::OnCreate()
	{}

	void Object::Start()
	{
		for (auto& script : m_attachedScripts | std::views::values)
		{
			script->OnStart(this);
		}
	}

	void Object::Render(sf::RenderWindow& window)
	{}

	void Object::Render(sf::RenderTexture& texture)
	{}

	void Object::Update()
	{
		for (auto& script : m_attachedScripts | std::views::values)
		{
			script->OnUpdate(this);
		}
	}

	const Vec2f& Object::GetPosition() const
	{
		return { 0, 0 };
	}

	void Object::SetPosition(const Vec2f& position)
	{
		const Vec2f& current_position = GetPosition();

		for (const std::string& child : m_children)
		{
			Object* obj = m_currentScene.GetSceneObject(child);
			obj->SetPosition(obj->GetPosition() + (position - current_position));
		}
	}

	float Object::GetRotation()
	{
		return -1;
	}

	void Object::SetRotation(float degrees)
	{
		const Vec2f& pos = GetPosition();
		float rad = (degrees - GetRotation()) * (std::numbers::pi_v<float> / 180.f);
		float c = cos(rad);
		float s = sin(rad);

		for (const std::string& child : m_children)
		{
			Object* obj = m_currentScene.GetSceneObject(child);

			Vec2f relative_position = obj->GetPosition() - pos;

			float newx = c * relative_position.x - s * relative_position.y + pos.x;
			float newy = s * relative_position.x + c * relative_position.y + pos.y;

			obj->SetPosition({ newx, newy});

			obj->SetRotation(degrees);
		}
	}

	const std::set<std::string>& Object::GetChildren()
	{
		return m_children;
	}

	std::vector<Object*> Object::GetChildrenAsObjects() const
	{
		std::vector<Object*> res {};
		res.reserve(m_children.size());

		for (const std::string& child_name : m_children)
		{
			Object* obj = m_currentScene.GetSceneObject(child_name);
			if (obj != nullptr)
			{
				res.push_back(obj);
			}
		}

		return res;
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
			Object* parent_obj = m_currentScene.GetSceneObject(m_parent);
			parent_obj->RemoveChild(name);
			m_parent.clear();
		}

		if (object != nullptr)
		{
			m_parent = object->name;
			object->AddChild(this);
		}
	}

	const std::string& Object::GetParent()
	{
		return m_parent;
	}

	void Object::Destroy(bool destroy_children)
	{
		Object* parent_obj = m_currentScene.GetSceneObject(m_parent);

		for (Object* child : GetChildrenAsObjects())
		{
			if (destroy_children)
			{
				child->Destroy(true);
			}
			else
			{
				child->SetParent(parent_obj);
			}
		}

		SetParent(nullptr);
		m_currentScene.RemoveFromScene(name);
	}

	json Object::SerializeScripts() const
	{
		json attached_scripts;
		for (const auto& it : m_attachedScripts)
		{
			const auto& reflection_vars = it.second->GetReflection().Get();
			const auto& bs = reflection_vars.b;
			const auto& flts = reflection_vars.flt;
			const auto& i8s = reflection_vars.i8;
			const auto& i16s = reflection_vars.i16;
			const auto& i32s = reflection_vars.i32;
			const auto& strs = reflection_vars.str;

			json bs_data;
			for (const auto& [name, val] : bs)
			{
				bs_data[name] = *val;
			}
			json flts_data;
			for (const auto& [name, val] : flts)
			{
				flts_data[name] = *val;
			}
			json i8s_data;
			for (const auto& [name, val] : i8s)
			{
				i8s_data[name] = *val;
			}
			json i16s_data;
			for (const auto& [name, val] : i16s)
			{
				i16s_data[name] = *val;
			}
			json i32s_data;
			for (const auto& [name, val] : i32s)
			{
				i32s_data[name] = *val;
			}
			json strs_data;
			for (const auto& [name, val] : strs)
			{
				strs_data[name] = *val;
			}
			attached_scripts[it.first] =
			{
				bs_data,
				flts_data,
				i8s_data,
				i16s_data,
				i32s_data,
				strs_data
			};
		}

		return attached_scripts;
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
