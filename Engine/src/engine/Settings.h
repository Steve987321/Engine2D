#pragma once

#include "EngineCore.h"

// important variables for the engine and can be used for the game

namespace Toad
{
    // current active directory workspace
    inline std::string game_project_directory = "D:\\VSProjects\\Engine2D\\bin\\x64\\Dev\\";

    #ifdef _WIN32
    inline std::string game_file_name = "Game.dll";
    #elif __APPLE__
    inline std::string game_file_name[MAX_PATH] = "libGame.dylib";
    #endif
}