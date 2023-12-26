#pragma once

namespace Toad
{

class ENGINE_API ResourceManager
{
public:
	ResourceManager() = default;

	std::unordered_map<std::string, sf::Texture>& GetTextures();
	sf::Texture* AddTexture(std::string_view id, const sf::Texture& tex);
	sf::Texture* GetTexture(std::string_view id);
	bool RemoveTexture(std::string_view id);

	void Clear();

private:
	std::unordered_map<std::string, sf::Texture> m_textures;
	//std::unordered_map<std::string, sf::Font> m_fonts;
};

}
