#pragma once

// Functions for the engine to read to get access to the game scripts and to register scripts

namespace Toad
{
	class Object;
}

#include "Engine/systems/ScriptManager.h"
#include "Game.h"

#include "engine/default_scripts/Script.h"
#include <functional>
#include <vector>

#define SCRIPT_REGISTER(T) register_script(new T(#T), sizeof(T))

struct ScriptBuffer
{
	ScriptBuffer(void* b, size_t s)
		: buf(b), size(s) 
	{}

	void* buf;
	size_t size;
};

struct Scripts
{
	ScriptBuffer* scripts;
	size_t len;
};

namespace Toad
{

extern "C"
{

// functions signatures
typedef void (register_scripts_t)();
typedef void (register_script_t)();
typedef Scripts (get_registered_scripts_t)();

GAME_API void register_scripts();
GAME_API void register_script(void* instance, size_t size);
GAME_API Scripts get_registered_scripts();

}

}
