#include "ScriptRegister.h"
#include "engine/Engine.h"

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

void register_script(void* instance, size_t size)
{
	get_registered_scripts().emplace_back(instance, size);
}

std::vector<SBuf>& get_registered_scripts()
{
	static std::vector<SBuf> scripts{};
	return scripts;
}

}
