#pragma once

// Functions for the engine to read to get access to the game scripts and to register scripts

#include <EngineCore.h>
#include "Game.h"

#include "engine/default_scripts/Script.h"

#define SCRIPT_REGISTER(T) register_script(new T(#T), sizeof(T))

struct SBuf
{
	SBuf(void* b, size_t s) : buf(b), size(s) {}
	void* buf;
	size_t size;
};

namespace Toad
{

extern "C"
{
	// functions signatures
	typedef void (register_scripts_t)();
	typedef void (register_script_t)();
	typedef std::vector<SBuf>& (get_registered_scripts_t)();

	GAME_API void register_scripts();
	GAME_API void register_script(void* instance, size_t size);
	GAME_API std::vector<SBuf>& get_registered_scripts();
}

}
