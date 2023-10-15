#pragma once

#include "EngineCore.h"

// important variables for the engine and can be used for the game

namespace Toad
{
    #ifdef _WIN32
    inline char game_file_name[MAX_PATH] = "Game.dll";
    #elif __APPLE__
    inline char game_file_name[MAX_PATH] = "libGame.dylib";
    #endif
}