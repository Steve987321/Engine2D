#include "pch.h"

#include "engine/Engine.h"

#include "ResourceManager.h"

namespace Toad
{

namespace ResourceManager
{
	// special case for textures as these can't be created before sfml initializes 
	static ResourcesOfType<sf::Texture> textures{ "Textures" };
	static ResourcesOfType<sf::Font> fonts{ "Fonts" };
	static ResourcesOfType<FSM> fsms{ "FSM" };
	static AudioSourceResources audio_sources{ "AudioSource" };

	static std::vector<std::any> default_resources;

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
		// set default resources 
		sf::Texture default_texture;
		sf::Font default_font; 
		bool resize_success = default_texture.resize({5, 5});
		assert(resize_success && "Can't resize default texture");
		bool load_success = default_font.openFromFile(DEFAULT_FONT_PATH);
		assert(load_success && "Can't load default font");

		// add them to their managers 
		textures.Add("Default", default_texture);
		fonts.Add("Default", default_font);
		audio_sources.Add("Default", {});
		fsms.Add("Default", {});
	}

}

}