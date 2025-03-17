#pragma once

#include "engine/default_objects/Object.h"

namespace Toad
{
	
struct ENGINE_API TextStyle
{
	sf::Text::Style style = sf::Text::Style::Regular;
	unsigned char_size = 30;
	float char_spacing = 0.5f;
	float line_spacing = 0.7f;
	sf::Color fill_col = {255, 255, 255};
	sf::Color outline_col = {0, 0, 0};
	float outline_thickness = 1.f;
};

ENGINE_API sf::Font GetDefaultFont();
ENGINE_API sf::Font& GetDefaultFontResource();

class ENGINE_API Text : public Object
{
public:
	Text(std::string_view obj_name);

public:
	void SetFont(const std::filesystem::path& relative_path, const sf::Font& font);
	const std::filesystem::path& GetFontPath();

	void SetText(std::string_view text);
	std::string GetText() const;

	void SetStyle(const TextStyle& style, bool apply = true);
	void ApplyStyle();
	const TextStyle& GetStyle() const;

	sf::Text& GetTextObj();

	void Start() override;
	void Render(sf::RenderTarget& target) override;
	void Update() override;
	const Vec2f& GetPosition() const override;
	void SetPosition(const Vec2f& position) override;

	void SetRotation(float degrees) override;
	float GetRotation() override;

	json Serialize() override;

private:
	TextStyle m_style;
	sf::Text m_text;
	std::filesystem::path m_font_path;
};

}
