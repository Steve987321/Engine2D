#pragma once

#include <EngineCore.h>

#include "engine/default_scripts/Script.h"

namespace Toad
{
	class Script;

///
/// An Object is the base class for different type of game objects
///	And holds scripts.
///
class Object
{
public:
	Object() = default;
	virtual ~Object() {}

	virtual void Start()
	{}

	virtual void Update(sf::RenderWindow& window)
	{}

	template <class T>
	T* GetScript()
	{
		for (int i = 0; i < m_scripts.size(); i++)
		{
			auto script = &m_scripts[i];

			auto res = dynamic_cast<T*>(script);
			if (res != nullptr)
				return res;
		}
		return nullptr;
	}

	template <class T>
	void AddScript()
	{
		for (int i = 0; i < m_scripts.size(); i++)
		{
			auto script = &m_scripts[i];

			auto res = dynamic_cast<T*>(script);
			if (res != nullptr)
			{
				//assert("Current object already has that type of script attached" && 0);
				return;
			}
		}

		m_scripts.emplace(T());
	}

	template <class T>
	void RemoveScript()
	{
		for (int i = 0 ; i < m_scripts.size(); i++)
		{
			auto& script = m_scripts[i];
			if (dynamic_cast<T*>(script) != nullptr)
			{
				m_scripts.erase(m_scripts.begin() + i);
				return;
			}
		}
		
	}

protected:
	// attached scripts 
	std::vector< Script > m_scripts {};

};

}
