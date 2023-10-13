#include "pch.h"
#include "game_core/ScriptRegister.h"
#include "TestScript.h"

#include "Engine/Engine.h"

using namespace Toad;
using namespace sf;

void TestScript::OnStart(Object* obj)
{
	Script::OnStart(obj);
	
	velx = start_direction_X;
	vely = start_direction_Y;
}

void TestScript::OnUpdate(Object* obj)
{
	Script::OnUpdate(obj);

	// sick dvd logo screen hitting script for circle objects 
	auto circle = dynamic_cast<Circle*>(obj); // TODO: thi s is shit 
	if (circle != nullptr)
	{
		auto& c = circle->GetCircle();

		auto win_size = Engine::Get().GetWindow().getSize();
		auto dt = Engine::Get().GetDeltaTime().asMilliseconds() / 1000.f;

		c.move(Vec2f{velx * dt, vely * dt} * speed_mult);
			
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
		{
			LOGDEBUG("PRESSED JUICY BUTTON");
		}
		const auto change_c_col = [&c]
		{
			auto rand_r = rand_int(0, 255);
			auto rand_g = rand_int(0, 255);
			auto rand_b = rand_int(0, 255);
			c.setFillColor(Color(rand_r, rand_g, rand_b));
		};

		auto c_bounds = c.getGlobalBounds();
		
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
	}
}

void TestScript::ExposeVars()
{
	Script::ExposeVars();

	EXPOSE_VAR(start_direction_X);
	EXPOSE_VAR(start_direction_Y);
	EXPOSE_VAR(speed_mult);
}
