#pragma once

namespace Toad
{

class Script;

using TGAME_SCRIPTS = std::unordered_map<std::string, Script*>;

namespace ScriptManager
{
	void LoadScripts();

	void CallUpdateScripts();
	void CallFixedUpdateScripts();
	void CallStartScripts();
};

}
