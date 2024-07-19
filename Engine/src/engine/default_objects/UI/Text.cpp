#include "pch.h"
#include "Text.h"

#include "engine/Engine.h"

namespace Toad
{

Text::Text(std::string_view obj_name)
{
	name = obj_name;
	
	sf::Font* default_font = ResourceManager::GetFonts().Get("Default");
	if (default_font == nullptr)
	{
		sf::Font arial;
		if (!arial.loadFromFile("C:\\Windows\\Fonts\\Arial.ttf"))
		{
			LOGWARN("Can't find C:\\Windows\\Fonts\\Arial.ttf");
			return;
		}

		default_font = ResourceManager::GetFonts().Add("Default", arial);
		SetFont("Default", *default_font);
	}
	else
	{
		SetFont("Default", *default_font);
	}
	m_style = TextStyle();
	ApplyStyle();
	SetText(obj_name);
}

void Text::SetFont(const std::filesystem::path& relative_path, const sf::Font& font)
{
	m_font_path = relative_path;
	m_text.setFont(font);
}

const std::filesystem::path& Text::GetFontPath()
{
	return m_font_path;
}

void Text::SetText(std::string_view text)
{
	m_text.setString(text.data());
}

std::string Text::GetText() const
{
	return m_text.getString();
}

void Text::SetStyle(const TextStyle& style, bool apply)
{
	m_style = style;

	if (apply)
	{
		ApplyStyle();
	}
}

void Text::ApplyStyle()
{
	m_text.setCharacterSize(m_style.char_size);
	m_text.setLetterSpacing(m_style.char_spacing);
	m_text.setLineSpacing(m_style.line_spacing);
	m_text.setOutlineThickness(m_style.outline_thickness);
	m_text.setFillColor(m_style.fill_col);
	m_text.setOutlineColor(m_style.outline_col);
	m_text.setStyle(m_style.style);
}

const TextStyle& Text::GetStyle() const
{
	return m_style;
}

sf::Text& Text::GetTextObj()
{
	return m_text;
}

void Text::Start()
{
	Object::Start();
}

void Text::Render(sf::RenderTarget& target)
{
	Object::Render(target);

	target.draw(m_text);
}

void Text::Update()
{
	Object::Update();
}

const Vec2f& Text::GetPosition() const
{
	return m_text.getPosition();
}

void Text::SetPosition(const Vec2f& position)
{
	Object::SetPosition(position);

	m_text.setPosition(position);
}

void Text::SetRotation(float degrees)
{
	Object::SetRotation(degrees);

	m_text.setRotation(degrees);
}

float Text::GetRotation()
{
	return m_text.getRotation();
}

json Text::Serialize()
{
	json data;
	json text_properties;
	json attached_scripts = SerializeScripts();

	text_properties["parent"] = GetParent();

	text_properties["posx"] = GetPosition().x;
	text_properties["posy"] = GetPosition().y;
	text_properties["rotation"] = GetRotation();

	text_properties["text"] = m_text.getString();
	text_properties["char_size"] = m_text.getCharacterSize();
	text_properties["char_spacing"] = m_text.getLetterSpacing();
	text_properties["line_spacing"] = m_text.getLineSpacing();
	text_properties["fill_col"] = m_text.getFillColor().toInteger();
	text_properties["outline_col"] = m_text.getOutlineColor().toInteger();
	text_properties["style"] = m_text.getStyle();
	text_properties["outline_thickness"] = m_text.getOutlineThickness();
	text_properties["font_loc"] = m_font_path;

	data["properties"] = text_properties;
	data["scripts"] = attached_scripts;

	return data;
}

}
