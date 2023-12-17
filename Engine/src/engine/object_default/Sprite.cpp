#include "pch.h"
#include "EngineMeta.h"
#include "EngineCore.h"
#include "Sprite.h"

namespace Toad
{

Sprite::Sprite(std::string_view obj_name)
{
	name = obj_name;
	auto default_tex = sf::Texture();
	default_tex.create(10, 10);
	m_sprite = sf::Sprite(default_tex);
}

Sprite::~Sprite()
{
}

sf::Sprite& Sprite::GetSprite()
{
	return m_sprite;
}

void Sprite::Render(sf::RenderWindow& window)
{
	// update attached scripts
	for (auto& script : m_attached_scripts | std::views::values)
	{
		script->OnUpdate(this);
	}

	// draw sfml sprite object
	window.draw(m_sprite);
}

void Sprite::Render(sf::RenderTexture& texture)
{
	// draw sfml sprite object
	texture.draw(m_sprite);
}

void Sprite::Update()
{
	// update attached scripts
	for (auto script : m_attached_scripts | std::views::values)
	{
		script->OnUpdate(this);
	}
}

}
