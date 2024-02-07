#include "pch.h"
#include "ScriptRegister.h"

#include "assets/scripts/TestScript.h"

// tools 
#include "engine/default_scripts/AnimationController.h"

namespace Toad
{

void register_scripts()
{
	get_registered_scripts().clear();

	SCRIPT_REGISTER(TestScript);

	SCRIPT_REGISTER(AnimationController);
}

void register_script(const std::shared_ptr<Script>& instance)
{
	get_registered_scripts().emplace_back(instance);
}

std::vector<std::shared_ptr<Script>>& get_registered_scripts()
{
	static std::vector<std::shared_ptr<Script>> scripts{};
	return scripts;
}

}
