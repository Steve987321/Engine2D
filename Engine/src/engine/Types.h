#pragma once

using Vec2i = sf::Vector2i;
using Vec2f = sf::Vector2f;
using Vec2u = sf::Vector2u;
using Vec3i = sf::Vector3i;
using Vec3f = sf::Vector3f;

struct AudioSource
{
	std::filesystem::path full_path;
	std::filesystem::path relative_path;
	sf::SoundBuffer sound_buffer;
	bool has_valid_buffer;
};
