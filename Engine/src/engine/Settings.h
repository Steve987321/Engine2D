#pragma once

#include "EngineCore.h"

// important variables for the engine and can be used for the game

namespace Toad
{
    #ifdef _WIN32
    inline wchar_t game_bin_path[MAX_PATH] = L"Game.dll";
    #elif __APPLE__
    inline char game_bin_path[MAX_PATH] = "libGame.dylib";
    #endif
}