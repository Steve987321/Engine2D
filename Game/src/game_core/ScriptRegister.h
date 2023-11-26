#pragma once

// Functions for the engine to read to get access to the game scripts and to register scripts

#include <EngineCore.h>
#include "Game.h"

#include "engine/default_scripts/Script.h"

#define SCRIPT_REGISTER(T) register_script(std::make_shared<T>(#T))

namespace Toad
{

extern "C"
{
	// functions signatures
	typedef void (register_scripts_t)();
	typedef void (register_script_t)();
	typedef std::vector<std::shared_ptr<Script>>& (get_registered_scripts_t)();

	GAME_API void register_scripts();
	GAME_API void register_script(const std::shared_ptr<Script>& instance);
	GAME_API std::vector<std::shared_ptr<Script>>& get_registered_scripts();
}

}
