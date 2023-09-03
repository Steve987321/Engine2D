#pragma once

// base of game which holds important data about the made game 

#include <EngineCore.h>

#include "default_scripts/Object.h"

#ifdef GAME_IS_EXPORT
#define GAME_API __declspec(dllexport)
#else
#define GAME_API __declspec(dllimport)
#endif

class GAME_API GameBase
{
public:
	GameBase();
	~GameBase();

private:
	sf::ContextSettings m_settings;

	std::vector<ScriptBase> m_scripts;
	std::vector<Object> m_instances;

};

