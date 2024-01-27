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

Sprite::~Sprite() = default;

sf::Sprite& Sprite::GetSprite()
{
	return m_sprite;
}

const Vec2f& Sprite::GetPosition()
{
	return m_sprite.getPosition();
}

void Sprite::SetPosition(const Vec2f& position)
{
	Object::SetPosition(position);

	m_sprite.setPosition(position);
}

void Sprite::SetRotation(float degrees)
{
	Object::SetRotation(degrees);

	m_sprite.setRotation(degrees);
}

float Sprite::GetRotation()
{
	return m_sprite.getRotation();
}

std::filesystem::path& Sprite::GetTextureSourcePath()
{
	return m_texture_source_path;
}

void Sprite::SetTexture(const std::filesystem::path& relative_path, const sf::Texture* texture)
{
	m_texture_source_path = relative_path;
	m_sprite.setTexture(*texture, true);
}

json Sprite::Serialize()
{
	json s_data;
	json sprite_properties;
	json attached_scripts = SerializeScripts();

	const auto& s = GetSprite();

	sprite_properties["parent"] = GetParent();

	sprite_properties["posx"] = GetPosition().x;
	sprite_properties["posy"] = GetPosition().y;
	sprite_properties["rotation"] = GetRotation();
	sprite_properties["fill_col"] = s.getColor().toInteger();
	sprite_properties["scalex"] = s.getScale().x;
	sprite_properties["scaley"] = s.getScale().y;
	bool has_texture = s.getTexture() != nullptr;
	if (has_texture)
	{
		sprite_properties["texture_loc"] = GetTextureSourcePath();
		sf::IntRect tex_rect = s.getTextureRect();
		json rect;
		rect["left"] = tex_rect.left;
		rect["top"] = tex_rect.top;
		rect["width"] = tex_rect.width;
		rect["heigth"] = tex_rect.height;

		sprite_properties["texture_rect"] = rect;
	}
	sprite_properties["has_texture"] = has_texture;

	s_data["properties"] = sprite_properties;
	s_data["scripts"] = attached_scripts;

	return s_data;
}

void Sprite::Render(sf::RenderWindow& window)
{
	// update attached scripts
	for (auto& script : m_attachedScripts | std::views::values)
	{
		script->OnUpdate(this);
	}

	window.draw(m_sprite);
}

void Sprite::Render(sf::RenderTexture& texture)
{
	texture.draw(m_sprite);
}

void Sprite::Start()
{
	Object::Start();
}

void Sprite::Update()
{
	Object::Update();
}

}
