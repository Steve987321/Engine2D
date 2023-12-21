#include "pch.h"

#include "engine/Engine.h"

#include "ResourceManager.h"

namespace Toad
{
	sf::Texture* ResourceManager::AddTexture(const sf::Texture& tex)
	{
		m_textures.emplace_back(tex);
		return &m_textures.back();
	}

	std::vector<sf::Texture>& ResourceManager::GetTextures()
	{
		return m_textures;
	}
}