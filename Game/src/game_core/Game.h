#pragma once

// Game data that holds important game information

#ifdef _WIN32
#define GAME_API __declspec(dllexport)
#else
#define GAME_API 
#endif

#include "engine/systems/Scene.h" 
#include "engine/Types.h"

struct AppSettings
{
	std::string window_name;
	int window_width;
	int window_height;
	int frame_limit;
	sf::ContextSettings ctx_settings;
};

inline std::vector<Toad::Scene> game_scenes{};
inline AppSettings game_settings
{
	"Test Game",
	1920, 
	1080,
	60,
	sf::ContextSettings()
};

extern "C"
{
	// functions signatures
	typedef std::vector<Toad::Scene> (get_game_scenes_t)();
	typedef AppSettings (get_game_settings_t)();

	GAME_API std::vector<Toad::Scene> get_scenes();
	GAME_API AppSettings get_game_settings();
}
