#pragma once

#include "EngineCore.h"

// important variables for the engine and can be used for the game

namespace Toad
{
    // temp: current active directory workspace
#ifdef TOAD_EDITOR
    inline std::string game_bin_directory = "D:\\VSProjects\\Engine2D\\bin\\Dev-windows-x86_64\\";
#else
    inline std::string game_bin_directory = "D:\\VSProjects\\Engine2D\\bin\\Release-windows-x86_64\\";
#endif
    #ifdef _WIN32
    inline std::string game_bin_file = "Game.dll";
    #elif __APPLE__
    inline std::string game_bin_file = "libGame.dylib";
    #endif
}