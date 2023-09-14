#include "pch.h"
#include "EngineMeta.h"
#include "EngineCore.h"
#include "Sprite.h"

namespace Toad
{

Sprite::Sprite(std::string_view obj_name)
{
	name = obj_name;
}

Sprite::~Sprite()
{
}

sf::Sprite& Sprite::GetSprite()
{
	return m_sprite;
}

void Sprite::Update(sf::RenderWindow& window)
{
	// update attached scripts
	for (auto& script : m_attached_scripts)
	{
		script.OnUpdate(this);
	}

	// draw sfml sprite object
	window.draw(m_sprite);
}

void Sprite::Update(sf::RenderTexture& texture)
{
	// update attached scripts
	for (auto& script : m_attached_scripts)
	{
		script.OnUpdate(this);
	}

	// draw sfml sprite object
	texture.draw(m_sprite);
}
}
