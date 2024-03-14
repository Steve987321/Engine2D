#pragma once

#include "EngineCore.h"

// important variables for the engine and can be used for the game

namespace Toad
{
    // temp: current active directory workspace
#ifdef __APPLE__
inline std::string game_bin_directory = std::filesystem::current_path();
#else
inline std::string game_bin_directory;
#endif

#ifdef _WIN32
inline std::string game_bin_file = "Game.dll";
#elif __APPLE__
inline std::string game_bin_file = "libGame.dylib";
#endif
}