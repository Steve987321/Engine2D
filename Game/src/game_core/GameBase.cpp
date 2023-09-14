#include "pch.h"
#include "GameBase.h"

#include "scripts/TestScript.h"

namespace Toad
{

GameBase::GameBase()
{
}

GameBase::~GameBase()
{
}

void GameBase::RegisterScripts()
{
	GetRegisteredScripts().clear();

	SCRIPT_REGISTER(TestScript);
}

std::vector<std::shared_ptr<Script>>& GameBase::GetRegisteredScripts()
{
	static std::vector<std::shared_ptr<Script>> scripts{};
	return scripts;
}

void GameBase::Register(std::shared_ptr<Script> instance)
{
	GetRegisteredScripts().emplace_back(instance);
}

}
