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
	std::string window_name;
	int window_width;
	int window_height;
	int frame_limit;
	uint32_t style;
};

extern "C"
{
	// functions signatures
	typedef AppSettings (get_game_settings_t)();

	GAME_API AppSettings get_game_settings();
}
