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

std::filesystem::path& Sprite::GetTextureSourcePath()
{
	return m_texture_source_path;
}

void Sprite::SetTexture(const std::filesystem::path& relative_path, const sf::Texture& texture)
{
	m_texture_source_path = relative_path;
	m_sprite.setTexture(texture, true);
}

json Sprite::Serialize()
{
	json s_data;
	json sprite_properties;
	json attached_scripts;

	for (const auto& it : GetAttachedScripts())
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
			bs_data[name] = *val;
		}
		json flts_data;
		for (const auto& [name, val] : flts)
		{
			flts_data[name] = *val;
		}
		json i8s_data;
		for (const auto& [name, val] : i8s)
		{
			i8s_data[name] = *val;
		}
		json i16s_data;
		for (const auto& [name, val] : i16s)
		{
			i16s_data[name] = *val;
		}
		json i32s_data;
		for (const auto& [name, val] : i32s)
		{
			i32s_data[name] = *val;
		}
		json strs_data;
		for (const auto& [name, val] : strs)
		{
			strs_data[name] = *val;
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

	const auto& s = GetSprite();

	sprite_properties["posx"] = s.getPosition().x;
	sprite_properties["posy"] = s.getPosition().y;
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
	for (auto& script : m_attached_scripts | std::views::values)
	{
		script->OnUpdate(this);
	}

	window.draw(m_sprite);
}

void Sprite::Render(sf::RenderTexture& texture)
{
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
