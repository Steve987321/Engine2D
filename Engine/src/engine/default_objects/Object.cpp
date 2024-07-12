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

	void Object::OnDestroy()
	{}

	void Object::Start()
	{
		for (auto& script : m_attachedScripts | std::views::values)
		{
			script->OnStart(this);
		}
	}

	void Object::Render(sf::RenderTarget& target)
	{
		for (auto& script : m_attachedScripts | std::views::values)
		{
			script->OnRender(this, target);
		}
	}
	
	void Object::Update()
	{
		for (auto& script : m_attachedScripts | std::views::values)
		{
			script->OnUpdate(this);
		}
	}

	void Object::FixedUpdate()
	{
		for (auto& script : m_attachedScripts | std::views::values)
		{
			script->OnFixedUpdate(this);
		}
	}

	void Object::LateUpdate()
	{
		for (auto& script : m_attachedScripts | std::views::values)
		{
			script->OnLateUpdate(this);
		}
	}

	void Object::End(Scene* next_scene)
	{
		for (auto& script : m_attachedScripts | std::views::values)
		{
			script->OnEnd(this, next_scene);
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
			Object* obj = m_currentScene.GetSceneObject(child).get();
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
			Object* obj = m_currentScene.GetSceneObject(child).get();

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
			Object* obj = m_currentScene.GetSceneObject(child_name).get();
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
			Object* parent_obj = m_currentScene.GetSceneObject(m_parent).get();
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
		OnDestroy();

		auto attached_scripts_temp = m_attachedScripts;
		for (const auto& it : attached_scripts_temp)
			RemoveScript(it.first);
		
		Object* parent_obj = m_currentScene.GetSceneObject(m_parent).get();
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
		m_currentScene.RemoveFromScene(name, Engine::Get().GameStateIsPlaying());
	}

	json Object::SerializeScripts() const
	{
		json attached_scripts;
		for (auto it : m_attachedScripts)
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
				if (val)
					bs_data[name] = *val;
				else
					LOGWARNF("[Object::SerializeScripts] variable {} is null", name);
			}
			json flts_data;
			for (const auto& [name, val] : flts)
			{
				if (val)
					flts_data[name] = *val;
				else
					LOGWARNF("[Object::SerializeScripts] variable {} is null", name);
			}
			json i8s_data;
			for (const auto& [name, val] : i8s)
			{
				if (val)
					i8s_data[name] = *val;
				else
					LOGWARNF("[Object::SerializeScripts] variable {} is null", name);
			}
			json i16s_data;
			for (const auto& [name, val] : i16s)
			{
				if (val)
					i16s_data[name] = *val;
				else
					LOGWARNF("[Object::SerializeScripts] variable {} is null", name);
			}
			json i32s_data;
			for (const auto& [name, val] : i32s)
			{
				if (val)
					i32s_data[name] = *val;
				else
					LOGWARNF("[Object::SerializeScripts] variable {} is null", name);
			}
			json strs_data;
			for (const auto& [name, val] : strs)
			{
				if (val)
					strs_data[name] = *val;
				else
					LOGWARNF("[Object::SerializeScripts] variable {} is null", name);
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

	Script* Object::GetScript(std::string_view name)
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
			//it->second->OnRemove(this);
			free(it->second);
			m_attachedScripts.erase(it);
			return true;
		}

		return false;
	}

	void Object::AddScript(Script* script)
	{
		m_attachedScripts[script->GetName()] = script;
		//script->OnAttach(this);
	}

	void Object::AddScript(std::string_view script_name)
	{
		const auto& gscripts = Engine::Get().GetGameScriptsRegister();
		if (auto it = gscripts.find(script_name.data()); it != gscripts.end())
		{
			AddScript(it->second->Clone());
			//return script_instance.get();
		}

		//return nullptr;
	}

	const std::unordered_map<std::string, Script*>& Object::GetAttachedScripts() const
	{
		return m_attachedScripts;
	}
}
