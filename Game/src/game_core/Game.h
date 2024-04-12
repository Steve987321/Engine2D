#pragma once

// Game data that holds important game information

#ifdef _WIN32
#define GAME_API __declspec(dllexport)
#else
#define GAME_API 
#endif

#include "pch.h"
#include <EngineCore.h>

struct AppSettings
{
	std::string window_name;
	int window_width;
	int window_height;
	int frame_limit;
	sf::ContextSettings ctx_settings;
};

inline AppSettings game_settings
{
	"Game",
	1280,
	720,
	60,
	sf::ContextSettings()
};

extern "C"
{
	// functions signatures
	typedef AppSettings (get_game_settings_t)();

	GAME_API AppSettings get_game_settings();
}
