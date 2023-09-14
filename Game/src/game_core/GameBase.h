#pragma once

// Base of game which holds important data about the made game 

#include <EngineCore.h>

#include "engine/default_scripts/Script.h"

#ifdef GAME_IS_EXPORT
#define GAME_API __declspec(dllexport)
#else
#define GAME_API __declspec(dllimport)
#endif

#define SCRIPT_CONSTRUCT(T)				 \
T(std::string_view name) : Script(name)	 \
{										 \
	m_name = name;						 \
}

#define SCRIPT_REGISTER(T) Register(std::make_shared<T>(#T))

namespace Toad
{

class GAME_API GameBase
{
public:
	GameBase();
	virtual ~GameBase();

	void RegisterScripts();
	static std::vector<std::shared_ptr<Script>>& GetRegisteredScripts();

protected:
	sf::ContextSettings m_settings;

private:
	void Register(std::shared_ptr<Script> instance);
};

}
