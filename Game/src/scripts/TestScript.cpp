#include "pch.h"
#include "game_core/ScriptRegister.h"
#include "TestScript.h"

#include "Engine/Engine.h"

using namespace Toad;
using namespace sf;

void TestScript::OnStart(Object* obj)
{
	Script::OnStart(obj);
	
	LOGDEBUG("{} {}", sizeof *this, sizeof Toad::Script);
	velx = start_direction_X;
	vely = start_direction_Y;
}

void TestScript::OnUpdate(Object* obj)
{
	Script::OnUpdate(obj);

	auto circle = dynamic_cast<Circle*>(obj);
	if (circle != nullptr)
	{
		auto& c = circle->GetCircle();

		auto win_size = Engine::Get().GetWindow().getSize();
		auto dt = Engine::Get().GetDeltaTime().asMilliseconds() / 1000.f;

		c.move(Vec2f{velx * dt, vely * dt} * speed_mult);

		const auto change_c_col = [&c]
		{
			auto rand_r = rand_int(0, 255);
			auto rand_g = rand_int(0, 255);
			auto rand_b = rand_int(0, 255);
			c.setFillColor(Color(rand_r, rand_g, rand_b));
		};

		auto c_bounds = c.getGlobalBounds();
		/*LOGDEBUG("c_bounds: left: {}, top: {}, widht: {}, height: {}",
			c_bounds.left, c_bounds.top, c_bounds.width, c_bounds.height);*/

		
		if (c_bounds.left < 0)
			if (velx < 0)
			{
				change_c_col();
				velx *= -1;
			}
		if (c_bounds.left + c_bounds.width > win_size.x)
			if (velx > 0)
			{
				change_c_col();
				velx *= -1;
			}
		if (c_bounds.top < 0)
			if (vely < 0)
			{
				change_c_col();
				vely *= -1;
			}
		if (c_bounds.top + c_bounds.height > win_size.y)
			if (vely > 0)
			{
				change_c_col();
				vely *= -1;
			}

		//static sf::Clock clock;
		
		//LOGDEBUG("TestScript: {}", clock.getElapsedTime().asMilliseconds());
	}
}

void TestScript::ExposeVars()
{
	Script::ExposeVars();

	EXPOSE_VAR(variable1);
	EXPOSE_VAR(start_direction_X);
	EXPOSE_VAR(start_direction_Y);
	EXPOSE_VAR(speed_mult);
}
