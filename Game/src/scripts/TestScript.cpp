#include "pch.h"
#include "game_core/ScriptRegister.h"
#include "TestScript.h"

#include "Engine/Engine.h"

using namespace Toad;

void TestScript::OnUpdate(Object* obj)
{
	Script::OnUpdate(obj);

	auto circle = dynamic_cast<Circle*>(obj);
	if (circle != nullptr)
	{
		circle->GetCircle().setFillColor(sf::Color::Blue);
		//static sf::Clock clock;
		
		//LOGDEBUG("TestScript: {}", clock.getElapsedTime().asMilliseconds());
	}
	obj->name = "Test Script is doing this";
}

void TestScript::ExposeVars()
{
	Script::ExposeVars();

	EXPOSE_VAR(variable1);
}
