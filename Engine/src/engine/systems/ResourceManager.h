#pragma once

#include "engine/Types.h"
#include "nlohmann/json.hpp"

namespace Toad
{

	// ids are relative paths
class ENGINE_API ResourceManager
{
public:
	ResourceManager();

	std::unordered_map<std::string, sf::Texture>& GetTextures();
	sf::Texture* AddTexture(std::string_view id, const sf::Texture& tex);
	sf::Texture* GetTexture(std::string_view id);
	bool RemoveTexture(std::string_view id);

	std::unordered_map<std::string, sf::Font>& GetFonts();
	sf::Font* AddFont(std::string_view id, const sf::Font& font);
	sf::Font* GetFont(std::string_view id);
	bool RemoveFont(std::string_view id);

	std::unordered_map<std::string, AudioSource>& GetAudioSources();
	AudioSource* AddAudioSource(std::string_view id, const AudioSource& sound_buffer);
	AudioSource* GetAudioSource(std::string_view id);
	// will only clear the soundbuffer and will still hold the rest of the audiosource data
	bool RemoveAudioSource(std::string_view id);

	void Clear();

private:
	std::unordered_map<std::string, sf::Texture> m_textures;
	std::unordered_map<std::string, AudioSource> m_audioSources;
	std::unordered_map<std::string, sf::Font> m_fonts;
};

}
