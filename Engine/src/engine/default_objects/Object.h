#pragma once

#include "EngineCore.h"

#include "engine/Types.h"
#include "nlohmann/json.hpp"
#include "engine/default_scripts/Script.h"

namespace Toad
{
	class Script;
	class Scene;

	using json = nlohmann::ordered_json;

///
/// An Object is the base class for different type of game objects
///	And holds scripts.
///
class ENGINE_API Object
{
public:
	std::string name;

	Object();
	explicit Object(std::string_view name);
	virtual ~Object();
	
	virtual void OnCreate();

	virtual void OnDestroy();

	virtual void Start();

	virtual void Render(sf::RenderTarget& target);

	virtual void Update();

	virtual void FixedUpdate();

	virtual void LateUpdate();

	virtual void End(Scene* next_scene);

	virtual const Vec2f& GetPosition() const;
	virtual void SetPosition(const Vec2f& position);

	virtual FloatRect GetBounds() const;

	virtual float GetRotation();
	virtual void SetRotation(float degrees);

	virtual json Serialize();

	const std::set<std::string>& GetChildren();
	std::vector<Object*> GetChildrenAsObjects() const;
	bool AddChild(Object* object);
	bool RemoveChild(std::string_view obj_name);

	void SetParent(Object* object);
	const std::string& GetParent();

	void Destroy(bool destroy_children = false);

	json SerializeScripts() const;

	void AddScript(Script* script);

	void AddScript(std::string_view script_name);

	Script* GetScript(std::string_view name);

	// faster than removing by script type
	bool RemoveScript(std::string_view script_name);

	// slower than removing by script string name
	template <class T>
	bool RemoveScript()
	{
		for (const auto& script : m_attachedScripts | std::views::values)
		{
			auto res = dynamic_cast<T*>(script);
			if (res != nullptr)
			{
				m_attachedScripts.erase(script->GetName());
				return true;
			}
		}

		return false;
	}

	template <class T>
	T* GetScript()
	{
		for (const auto& script : m_attachedScripts | std::views::values)
		{
			auto res = dynamic_cast<T*>(script);
			if (res != nullptr)
				return res;
		}

		return nullptr;
	}

	template <class T>
	T* GetScript(std::string_view name)
	{
		auto it = m_attachedScripts.find(name.data());
		if (it != m_attachedScripts.end())
		{
			return dynamic_cast<T*>(it->second);
		}

		return nullptr;
	}

	const std::unordered_map<std::string, Script* >& GetAttachedScripts() const;

protected:
	// attached scripts 
	std::unordered_map<std::string, Script* > m_attachedScripts;

	std::set<std::string> m_children{};
	std::string m_parent;

	Scene& m_currentScene;

private:
	// position used only by Object
	Vec2f m_objectPos{0, 0};

};

}
