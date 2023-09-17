#include "pch.h"
#include "game_core/GameBase.h"
#include "TestScript.h"

using namespace Toad;

void TestScript::OnUpdate(Object* obj)
{
	Script::OnUpdate(obj);

	std::cout << "this is a test\n";
}
