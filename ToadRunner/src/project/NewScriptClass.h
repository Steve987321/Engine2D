#pragma once

// #TODO store in file
namespace Toad
{
	// #TODO make framework/Framework.h relative
constexpr const char* NewScriptCplusplus =
R"(#include "pch.h"
#include "framework/Framework.h"
#include "$0@.h"

using namespace Toad;

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

// Show variables in the editor and serialize to scene
void $0@::ExposeVars()
{
	Script::ExposeVars();

}
)";

constexpr const char* NewScriptHeader =
R"(#pragma once

#include "engine/Engine.h"

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