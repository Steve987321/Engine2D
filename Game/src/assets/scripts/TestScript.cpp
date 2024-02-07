#include "pch.h" 
#include "framework/Framework.h"
#include "TestScript.h"

#include "Engine/Engine.h"

using namespace Toad;
using namespace sf;


Circle* circle;

void TestScript::OnStart(Object* obj)
{
	Script::OnStart(obj);

	circle = dynamic_cast<Circle*>(obj);

	if (circle == nullptr)
	{
		LOGERRORF("CIRCLE IS NULL");
	}

	velx = start_direction_X;
	vely = start_direction_Y;
}

void TestScript::OnUpdate(Object* obj)
{
	Script::OnUpdate(obj);

	// sick dvd logo screen hitting script for circle objects 
	if (circle != nullptr)
	{
		float dt = static_cast<float>(Engine::Get().GetDeltaTime().asMilliseconds()) / 1000.f;

		const auto change_c_col = []
			{
				auto rand_r = rand_int(0, 255);
				auto rand_g = rand_int(0, 255);
				auto rand_b = rand_int(0, 255);
				circle->GetCircle().setFillColor(Color(rand_r, rand_g, rand_b));
			};

		auto c_bounds = circle->GetCircle().getGlobalBounds();

		if (Camera::GetActiveCamera())
		{
			auto campos = Camera::GetActiveCamera()->GetPosition();
			auto camsize = Camera::GetActiveCamera()->GetSize();
			auto camposlefttop = Vec2f{ campos.x - camsize.x / 2.f, campos.y - camsize.y / 2.f };

			if (c_bounds.left < camposlefttop.x)
				if (velx < 0)
				{
					change_c_col();
					velx *= -1;
				}
			if (c_bounds.left + c_bounds.width > camposlefttop.x + camsize.x / 2)
				if (velx > 0)
				{
					change_c_col();
					velx *= -1;
				}
			if (c_bounds.top < camposlefttop.y)
				if (vely < 0)
				{
					change_c_col();
					vely *= -1;
				}
			if (c_bounds.top + c_bounds.height > camposlefttop.y + camsize.y / 2)
				if (vely > 0)
				{
					change_c_col();
					vely *= -1;
				}
		}

		circle->SetPosition(circle->GetPosition() + Vec2f{velx * dt, vely * dt} * speed_mult);
	}
}


void TestScript::ExposeVars()
{
	Script::ExposeVars();

	EXPOSE_VAR(start_direction_X);
	EXPOSE_VAR(start_direction_Y);
	EXPOSE_VAR(speed_mult);
}
