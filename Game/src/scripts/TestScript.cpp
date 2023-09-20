#include "pch.h"
#include "game_core/ScriptRegister.h"
#include "TestScript.h"

#include "Engine/Engine.h"

using namespace Toad;
using namespace sf;

void TestScript::OnCreate(Object* obj)
{
	Script::OnCreate(obj);


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
		
		static Vec2f vel = {
			start_direction_X * dt,
			start_direction_Y * dt
		};

		c.move(vel * speed_mult);

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
			if (vel.x < 0)
			{
				change_c_col();
				vel.x *= -1;
			}
		if (c_bounds.left + c_bounds.width > win_size.x)
			if (vel.x > 0)
			{
				change_c_col();
				vel.x *= -1;
			}
		if (c_bounds.top < 0)
			if (vel.y < 0)
			{
				change_c_col();
				vel.y *= -1;
			}
		if (c_bounds.top + c_bounds.height > win_size.y)
			if (vel.y > 0)
			{
				change_c_col();
				vel.y *= -1;
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
