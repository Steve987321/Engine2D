#pragma once

#include "EngineCore.h"
#include "engine/Types.h"
#include "engine/utils/DLib.h"

namespace Toad
{

class Script;

namespace ScriptManager
{
	ENGINE_API void RegisterScript(void* script, size_t size, TGAME_SCRIPTS& scripts);

	ENGINE_API void LoadScripts();
	ENGINE_API void SetBinPaths(std::string_view game_bin_file_name, std::string_view bin_path);
	
	ENGINE_API void CallStartScripts();
	ENGINE_API void CallUpdateScripts();
	ENGINE_API void CallFixedUpdateScripts();

    ENGINE_API DllHandle GetDLLHandle();
	TGAME_SCRIPTS& GetScripts();
};

}
