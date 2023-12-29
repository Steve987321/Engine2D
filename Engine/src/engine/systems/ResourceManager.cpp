#include "pch.h"

#include "engine/Engine.h"

#include "ResourceManager.h"

namespace Toad
{
	sf::Texture* ResourceManager::AddTexture(std::string_view id, const sf::Texture& tex)
	{
		if (m_textures.contains(id.data()))
		{
			LOGWARNF("[ResourceManager] Texture {} already exists and is getting replaced", id.data());
		}
		m_textures[id.data()] = sf::Texture(tex);
		return &m_textures[id.data()];
	}

	sf::Texture* ResourceManager::GetTexture(std::string_view id)
	{
		if (m_textures.contains(id.data()))
		{
			return &m_textures[id.data()];
		}
		return nullptr;
	}

	bool ResourceManager::RemoveTexture(std::string_view id)
	{
		if (auto it = m_textures.find(id.data()); 
			it != m_textures.end())
		{
			m_textures.erase(it);
			return true;
		}

		return false;
	}

	void ResourceManager::Clear()
	{
		m_textures.clear();
	}

	std::unordered_map<std::string, sf::Texture>& ResourceManager::GetTextures()
	{
		return m_textures;
	}
}