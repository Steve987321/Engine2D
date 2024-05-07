#pragma once

namespace Toad
{

class Script;

using TGAME_SCRIPTS = std::unordered_map<std::string, Script*>;

class ScriptManager
{
public: 

private:

	TGAME_SCRIPTS m_scripts;

};

}
