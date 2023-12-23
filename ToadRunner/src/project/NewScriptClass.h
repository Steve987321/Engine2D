#pragma once

namespace Toad
{
constexpr const char* NewScriptCplusplus =
R"(#include "pch.h"
#include "game_core/ScriptRegister.h"
#include "$0@.h"

#include "Engine/Engine.h"

using namespace Toad;
using namespace sf;

// Called on scene begin 
void $0@::OnStart(Object* obj)
{
	Script::OnStart(obj);

}

// Called every frame
void $0@::OnUpdate(Object* obj)
{
	Script::OnUpdate(obj);

}

// Show variables in the editor
void $0@::ExposeVars()
{
	Script::ExposeVars();

}
)";

constexpr const char* NewScriptHeader =
R"(#pragma once

class GAME_API $0@ : public Toad::Script
{
public:
	SCRIPT_CONSTRUCT($0@)

	void OnStart(Toad::Object* obj) override;
	void OnUpdate(Toad::Object* obj) override;
	void ExposeVars() override;
};

)";

}