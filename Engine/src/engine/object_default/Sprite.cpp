#include "pch.h"
#include "EngineMeta.h"
#include "EngineCore.h"
#include "Sprite.h"

namespace Toad
{

Sprite::Sprite()
{
	s_spriteInstances.emplace(this);
}

Sprite::~Sprite()
{
	s_spriteInstances.erase(this);
}

sf::Sprite& Sprite::GetSprite()
{
	return m_sprite;
}

void Sprite::Update(sf::RenderWindow& window)
{
	// update attached scripts
	for (auto& script : m_scripts)
	{
		script.OnUpdate();
	}

	// draw sfml sprite object
	window.draw(m_sprite);
}

}