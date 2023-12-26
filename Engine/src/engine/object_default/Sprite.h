#pragma once

#include "Object.h"

namespace Toad
{

class ENGINE_API Sprite : public Object
{
public:
	explicit Sprite(std::string_view obj_name);
	~Sprite() override;

	sf::Sprite& GetSprite();
	std::filesystem::path& GetTextureSourcePath();
	void SetTexture(const std::filesystem::path& relative_path, const sf::Texture& texture);
	json Serialize();

private:
	void Render(sf::RenderWindow& window) override;
	void Render(sf::RenderTexture& texture) override;
	void Update() override;

	sf::Sprite m_sprite;
	std::filesystem::path m_texture_source_path;
};

}
