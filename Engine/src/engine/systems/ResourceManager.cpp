#include "pch.h"

#include "engine/Engine.h"

#include "ResourceManager.h"

namespace Toad
{

namespace ResourceManager
{
	static ResourcesOfType<sf::Texture> textures{ "Texture" };
	static ResourcesOfType<sf::Font> fonts{ "Font" };
	static ResourcesOfType<FSM> fsms{ "FSM" };
	static AudioSourceResources audio_sources{ "AudioSource" };

	bool AudioSourceResources::Remove(std::string_view id)
	{
		if (auto it = m_data.find(id.data());
			it != m_data.end())
		{
			it->second.has_valid_buffer = false;
			it->second.sound_buffer = {};
			return true;
		}

		return false;
	}

	ResourcesOfType<sf::Texture>& GetTextures()
	{
		return textures;
	}

	ResourcesOfType<sf::Font>& GetFonts()
	{
		return fonts;
	}

	AudioSourceResources& GetAudioSources()
	{
		return audio_sources;
	}

	ResourcesOfType<FSM>& GetFSMs()
	{
		return fsms;
	}

	void Clear()
	{
		textures.Clear();
		audio_sources.Clear();
		fonts.Clear();
		fsms.Clear();
	}

	void Init()
	{
		bool resize_success = textures.default_resource.resize({5, 5});
		assert(resize_success && "Can't resize default texture");
		bool load_success = fonts.default_resource.openFromFile(DEFAULT_FONT_PATH);
		assert(load_success && "Can't load default font");
	}

}

}