#pragma once

#include "EngineCore.h"

// important variables for the engine and can be used for the game

namespace Toad
{
    #if WIN32
    inline char game_bin_path[MAX_PATH] = L"Game.dll";
    #elif __APPLE__
    inline char game_bin_path[MAX_PATH] = "libGame.dylib";
    #endif
}