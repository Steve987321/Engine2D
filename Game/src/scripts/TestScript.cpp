#include "pch.h"
#include "game_core/GameBase.h"
#include "TestScript.h"

#include "engine/object_default/Object.h"

using namespace Toad;

void TestScript::OnUpdate(Object* obj)
{
	Script::OnUpdate(obj);

	obj->name = "Test Script is doing this";
}
