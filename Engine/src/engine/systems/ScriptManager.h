#pragma once

#include "EngineCore.h"
#include "engine/Types.h"

namespace Toad
{

class Script;

namespace ScriptManager
{
	void ENGINE_API RegisterScript(void* script, size_t size, TGAME_SCRIPTS& scripts);

	void ENGINE_API LoadScripts();
	void ENGINE_API SetBinPaths(std::string_view game_bin_file_name, std::string_view bin_path);
	
	void ENGINE_API CallStartScripts();
	void ENGINE_API CallUpdateScripts();
	void ENGINE_API CallFixedUpdateScripts();

	TGAME_SCRIPTS& GetScripts();
};

}
