#include "pch.h"

#include "engine/Engine.h"
#include "engine/systems/ecs/EntityReg.h"

#include "Entity.h"

namespace Toad
{
	Entity::Entity(uint32_t id)
	{
		m_entities.emplace(this);
		m_id = id;
	}

	Entity::~Entity()
	{
		m_entities.erase(this);
	}

	uint32_t Entity::GetID() const
	{
		return m_id;
	}
}
