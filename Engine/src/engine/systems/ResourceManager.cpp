#include "pch.h"

#include "engine/Engine.h"

#include "ResourceManager.h"

namespace Toad
{
	ResourceManager::ResourceManager() = default;

	std::unordered_map<std::string, sf::Texture>& ResourceManager::GetTextures()
	{
		return m_textures;
	}

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

	std::unordered_map<std::string, sf::Font>& ResourceManager::GetFonts()
	{
		return m_fonts;
	}

	sf::Font* ResourceManager::AddFont(std::string_view id, const sf::Font& font)
	{
		if (m_fonts.contains(id.data()))
		{
			LOGWARNF("[ResourceManager] Font {} already exists and is getting replaced", id.data());
		}
		m_fonts[id.data()] = sf::Font(font);
		return &m_fonts[id.data()];
	}

	sf::Font* ResourceManager::GetFont(std::string_view id)
	{
		if (m_fonts.contains(id.data()))
		{
			return &m_fonts[id.data()];
		}
		return nullptr;
	}

	bool ResourceManager::RemoveFont(std::string_view id)
	{
		if (auto it = m_fonts.find(id.data());
			it != m_fonts.end())
		{
			m_fonts.erase(it);
			return true;
		}

		return false;
	}

	std::unordered_map<std::string, AudioSource>& ResourceManager::GetAudioSources()
	{
		return m_audioSources;
	}

	AudioSource* ResourceManager::AddAudioSource(std::string_view id, const AudioSource& sound_buffer)
	{
		if (m_audioSources.contains(id.data()))
		{
			LOGWARNF("[ResourceManager] AudioSource {} already exists and is getting replaced", id.data());
		}
		m_audioSources[id.data()] = AudioSource(sound_buffer);
		return &m_audioSources[id.data()];
	}

	AudioSource* ResourceManager::GetAudioSource(std::string_view id)
	{
		if (m_audioSources.contains(id.data()))
		{
			return &m_audioSources[id.data()];
		}
		return nullptr;
	}

	bool ResourceManager::RemoveAudioSource(std::string_view id)
	{
		if (auto it = m_audioSources.find(id.data());
			it != m_audioSources.end())
		{
			it->second.has_valid_buffer = false;
			it->second.sound_buffer = {};
			return true;
		}

		return false;
	}

	std::unordered_map<std::string, FSM>& ResourceManager::GetFSMs()
	{
		return m_fsms;
	}

	FSM* ResourceManager::AddFSM(std::string_view id, const FSM& fsm)
	{
		if (m_fsms.contains(id.data()))
			LOGWARNF("[ResourceManager] FSM {} already exists and is getting replaced", id.data());

		m_fsms[id.data()] = FSM(fsm);
		return &m_fsms[id.data()];
	}

	FSM* ResourceManager::GetFSM(std::string_view id)
	{
		if (m_fsms.contains(id.data()))
		{
			return &m_fsms[id.data()];
		}
		return nullptr;
	}

	bool ResourceManager::RemoveFSM(std::string_view id)
	{
		if (auto it = m_fsms.find(id.data());
			it != m_fsms.end())
		{
			m_fsms.erase(it);
			return true;
		}

		return false;
	}

	void ResourceManager::Clear()
	{
		m_textures.clear();
		m_audioSources.clear();
		m_fonts.clear();
		m_fsms.clear();
	}
}