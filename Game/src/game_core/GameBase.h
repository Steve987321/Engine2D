#pragma once

// Base of game which holds important data about the made game 

#include <EngineCore.h>

#include "engine/default_scripts/Script.h"

#define GAME_API __declspec(dllexport)

#define SCRIPT_CONSTRUCT(T)				 \
T(std::string_view name) : Script(name)	 \
{										 \
	m_name = name;						 \
}

#define SCRIPT_REGISTER(T) register_script(std::make_shared<T>(#T))

namespace Toad
{

extern "C"
{
	typedef void (register_scripts_t)();
	typedef void (register_script_t)();
	typedef std::vector<std::shared_ptr<Script>>& (get_registered_scripts_t)();

	GAME_API void register_scripts();
	GAME_API void register_script(const std::shared_ptr<Script>& instance);
	GAME_API std::vector<std::shared_ptr<Script>>& get_registered_scripts();
}

}
