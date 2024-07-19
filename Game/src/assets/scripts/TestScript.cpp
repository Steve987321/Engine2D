#include "framework/Framework.h"
#include "TestScript.h"

#include "Engine/Engine.h"

using namespace Toad;


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
		float dt = Time::GetDeltaTime();

		const auto change_c_col = [this]
			{
				auto rand_r = rand_int(0, 255);
				auto rand_g = rand_int(0, 255);
				auto rand_b = rand_int(0, 255);
				circle->GetCircle().setFillColor(sf::Color(rand_r, rand_g, rand_b));
			};

		sf::FloatRect c_bounds = circle->GetCircle().getGlobalBounds();

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
			if (c_bounds.left + c_bounds.width > camposlefttop.x + camsize.x)
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
			if (c_bounds.top + c_bounds.height > camposlefttop.y + camsize.y)
				if (vely > 0)
				{
					change_c_col();
					vely *= -1;
				}
		}

		circle->SetPosition(circle->GetPosition() + Vec2f{ velx, vely } *speed_mult * dt);
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
void TestScript::OnEditorUI(Toad::Object* obj, ImGuiContext* ctx)
{
	ImGui::SetCurrentContext(ctx);

	ImGui::Text("Velocity: %f %f", velx, vely);
}
#endif

#if defined(TOAD_EDITOR) || !defined(NDEBUG)
void TestScript::OnImGui(Toad::Object* obj, ImGuiContext* ctx)
{
	ImGui::SetCurrentContext(ctx);

	ImGui::Begin("[TestScript] Controller menu");

	static int fps = 60;
	static float fixed_time_step = Time::fixed_delta_time;
	ImGui::DragFloat("Ball Speed Mult", &speed_mult);
	if (ImGui::DragInt("FPS Lock", &fps))
		Engine::Get().GetWindow().setFramerateLimit(fps);
	
	if (ImGui::InputFloat("Fixed Step Delta", &fixed_time_step, 1.0e-7f, 5.f, "%.7f"))
		Time::fixed_delta_time = std::clamp(fixed_time_step, 1.0e-7f, 5.f);

	ImGui::SeparatorText("Info");
	if (circle)
		ImGui::Text("Ball position: (%f, %f)", circle->GetPosition().x, circle->GetPosition().y);
	else 
		ImGui::Text("Ball position: ?");
		
	ImGui::Text("Frame per sec: %f", 1.f / Time::GetDeltaTime());

	ImGui::End();
}
#endif 
