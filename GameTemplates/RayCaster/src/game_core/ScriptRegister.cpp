#include "ScriptRegister.h"

#include "assets/scripts/TestScript.h"

// tools 
//#include "engine/default_scripts/AnimationController.h"
#include "assets/scripts/Map.h"
#include "assets/scripts/RayCaster.h"
#include "assets/scripts/Player.h"
#include "assets/scripts/MapObject.h"

namespace Toad
{

static std::vector<ScriptBuffer> scripts{};

void register_scripts()
{
	scripts.clear();

	//SCRIPT_REGISTER(AnimationController);
	SCRIPT_REGISTER(Map);
	SCRIPT_REGISTER(RayCaster);
	SCRIPT_REGISTER(Player);
	SCRIPT_REGISTER(MapObject);
}

void register_script(void* instance, size_t size)
{
	scripts.emplace_back(instance, size);
}

Scripts get_registered_scripts()
{
	return {scripts.data(), scripts.size()};
}

}
