#pragma once

namespace Toad
{

class ResourceManager
{
public:
	ResourceManager() = default;

	std::vector<sf::Texture>& GetTextures();
	sf::Texture* AddTexture(const sf::Texture& tex);

private:
	std::vector<sf::Texture> m_textures;
};

}
