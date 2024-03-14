#include "framework/Framework.h"
#include "TestScript.h"

#include "Engine/Engine.h"

using namespace Toad;
using namespace sf;


void TestScript::OnStart(Object* obj)
{
	Script::OnStart(obj);

	circle = dynamic_cast<Circle*>(obj);

	if (circle == nullptr)
	{
		LOGERROR("CIRCLE IS NULL");
	}

	velx = start_direction_X;
	vely = start_direction_Y;
}

void TestScript::OnUpdate(Object* obj)
{
	Script::OnUpdate(obj);
}


void TestScript::OnFixedUpdate(Toad::Object* obj)
{
	if (circle != nullptr)
	{
		float dt = Engine::Get().GetDeltaTime().asSeconds();

		const auto change_c_col = [this]
			{
				auto rand_r = rand_int(0, 255);
				auto rand_g = rand_int(0, 255);
				auto rand_b = rand_int(0, 255);
				circle->GetCircle().setFillColor(Color(rand_r, rand_g, rand_b));
			};

		FloatRect c_bounds = circle->GetCircle().getGlobalBounds();

		if (Camera::GetActiveCamera())
		{
			const Vec2f& campos = Camera::GetActiveCamera()->GetPosition();
			const Vec2f camsize = Camera::GetActiveCamera()->GetSize();
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

		circle->SetPosition(circle->GetPosition() + Vec2f{ velx * dt, vely * dt } *speed_mult);
	}
}

void TestScript::ExposeVars()
{
	Script::ExposeVars();

	EXPOSE_VAR(start_direction_X);
	EXPOSE_VAR(start_direction_Y);
	EXPOSE_VAR(speed_mult);
}

#ifdef TOAD_EDITOR
void TestScript::OnEditorUI(ImGuiContext* ctx)
{
	ImGui::SetCurrentContext(ctx);

	ImGui::Text("Velocity: %f %f", velx, vely);
}
#endif // TOAD_EDITOR

