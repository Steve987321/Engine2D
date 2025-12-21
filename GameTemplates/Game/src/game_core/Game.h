#pragma once

// Game data that holds important game information

#ifdef _WIN32
#define GAME_API __declspec(dllexport)
#else
#define GAME_API __attribute__((__visibility__("default")))
#endif

#include "pch.h"
#include "EngineCore.h"

struct AppSettings
{
	std::string window_name = "Game";
	int window_width = 1280;
	int window_height = 720;
	int frame_limit = 60;
	uint32_t style = sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize;
};

extern "C"
{
	// functions signatures
	typedef AppSettings (get_game_settings_t)();

	GAME_API AppSettings get_game_settings();
}
