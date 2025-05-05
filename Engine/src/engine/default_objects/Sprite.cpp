#include "pch.h"
#include "EngineMeta.h"
#include "EngineCore.h"
#include "Sprite.h"

namespace Toad
{

Sprite::Sprite(std::string_view obj_name)
	: m_sprite(*ResourceManager::GetTextures().Get("Default"))
{
	name = obj_name;    
    last_scale = m_sprite.getScale();
}

Sprite::~Sprite() = default;

bool Sprite::HasDefaultTexture()
{
    return has_default_texture;
}

sf::Sprite& Sprite::GetSprite()
{
	return m_sprite;
}

const Vec2f& Sprite::GetPosition() const
{
    return m_objectPos;
}

void Sprite::SetPosition(const Vec2f& position)
{
	Object::SetPosition(position);
	m_sprite.setPosition(position);
}

FloatRect Sprite::GetBounds() const
{
	return FloatRect(m_sprite.getGlobalBounds());
}

void Sprite::SetRotation(float degrees)
{
	Object::SetRotation(degrees);
	m_sprite.setRotation(sf::degrees(degrees));
}

float Sprite::GetRotation()
{
	return m_sprite.getRotation().asDegrees();
}

const Vec2f& Sprite::GetScale() const
{
    return last_scale;
}

void Sprite::SetScale(const Vec2f& scale)
{
    last_scale = scale;
	m_sprite.setScale(scale);
}

std::filesystem::path& Sprite::GetTextureSourcePath()
{
	return m_texture_source_path;
}

void Sprite::SetTexture(const std::filesystem::path& relative_path, const sf::Texture* texture)
{
	if (relative_path != "Default")
		has_default_texture = false;
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
	sprite_properties["originx"] = s.getOrigin().x;
	sprite_properties["originy"] = s.getOrigin().y;

	sprite_properties["texture_loc"] = GetTextureSourcePath();
	sf::IntRect tex_rect = s.getTextureRect();
	json rect;
	rect["left"] = tex_rect.position.x;
	rect["top"] = tex_rect.position.y;
	rect["width"] = tex_rect.size.x;
	rect["height"] = tex_rect.size.y;

	sprite_properties["texture_rect"] = rect;

	s_data["properties"] = sprite_properties;
	s_data["scripts"] = attached_scripts;

	return s_data;
}

void Sprite::Render(sf::RenderTarget& target)
{
	target.draw(m_sprite);
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
