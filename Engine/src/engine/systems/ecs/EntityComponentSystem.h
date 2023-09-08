#pragma once

#include "engine/systems/ecs/Entity.h"
#include "entities_default/Sprite.h"

#include <unordered_map>
#include <set>

namespace Toad
{

///
/// ecs for each component type for any entity
///
template <class T>
class EntityComponentSystem
{
public:
	friend class ECSManager;

	EntityComponentSystem()
	{
		s_ecsInstances.emplace_back(this);
	}
	~EntityComponentSystem()
	{
	}

	Entity CreateEntity()
	{
		return ++m_entities;
	}

	Sprite CreateSprite()
	{
		Sprite sprite(++m_entities, sf::Sprite());
		return sprite;
	}

	Entity GetEntitySize() const
	{
		return m_entities;
	}

	void AddComponent(const Entity& e, const T& component)
	{
		auto id = e.GetID();
		auto it = m_entityComponents.find();
		if (it != m_entityComponents.end())
		{
			// entity already has components
			it->second.emplace_back(component);
		}
		else
		{
			// entity doesn't have any components
			m_entityComponents.insert({ id, {component} });
		}
	}

private:
	Entity m_entities = 0;

	std::unordered_map< uint32_t, std::set<T> > m_entityComponents{};

	static std::vector<EntityComponentSystem*> s_ecsInstances;
};

class ECSManager
{
public:
	void AddComponent(Entity& e); 

public:
	//std::vector<EntityComponentSystem<>> m_ecsRB;
};

}
