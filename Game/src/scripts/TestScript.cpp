#include "pch.h"
#include "game_core/ScriptRegister.h"
#include "TestScript.h"

#include "Engine/Engine.h"

using namespace Toad;
using namespace sf;

void TestScript::OnUpdate(Object* obj)
{
	Script::OnUpdate(obj);

	auto circle = dynamic_cast<Circle*>(obj);
	if (circle != nullptr)
	{
		circle->GetCircle().setFillColor(Color::Blue);
		//static sf::Clock clock;
		
		//LOGDEBUG("TestScript: {}", clock.getElapsedTime().asMilliseconds());
	}
}

void TestScript::ExposeVars()
{
	Script::ExposeVars();

	EXPOSE_VAR(variable1);
}
