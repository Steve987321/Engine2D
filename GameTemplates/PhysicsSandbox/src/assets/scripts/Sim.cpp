#include "framework/Framework.h"
#include "Sim.h"
#include "Fizzix/FZWorld.h"
#include "Fizzix/FZMath.h"

#include "implot/implot.h"

#include "Toot/Compiler/Compiler.h"
#include "Toot/TVM/TVM.h"
#include "Toot/Compiler/Parser.h" // op codes to string 
#include "Toot/TVM/Lib/SimLib.h"
#include "Toot/TVM/Lib/IO.h"

#include "engine/Engine.h"
#include "engine/PlaySession.h"
#include "engine/systems/Timer.h"

#include "UI/FizzixMenu.h"
#include "UI/TVMMenu.h"

#include "SimEnvironments/CarEnvironment.h"
#include "SimEnvironments/StressTestEnvironment.h"

using namespace Toad;

static VM vm;
static fz::World world;
static bool run_vm = false;

static bool lmouse_released = false;
static bool lmouse_pressed = false;
static bool rmouse_released = false;
static bool rmouse_pressed = false;

static bool add_potential_spring = false;
static bool add_potential_square = false;

static float sim_time = 0.f;
static float vm_time = 0.f;
static float fixed_time = 0.f;

static void OnMousePress(sf::Mouse::Button mouse)
{
	if (mouse == sf::Mouse::Button::Left)
		lmouse_pressed = true; 
	else if (mouse == sf::Mouse::Button::Right)
		rmouse_pressed = true; 
}

static void OnMouseRelease(sf::Mouse::Button mouse)
{
	if (mouse == sf::Mouse::Button::Left)
		lmouse_released = true;
	else if (mouse == sf::Mouse::Button::Right)
		rmouse_released = true; 
}

fz::World& Sim::GetWorld()
{
    return world;
}

void Sim::SetDefaultWorld(fz::World& world)
{
	world.ResetRenderingState();

	std::array<Vec2f, 6> player_vertices = fz::CreateSquare(30, 50);
	std::array<Vec2f, 6> floor_vertices = fz::CreateSquare(100000, 500);

	fz::Polygon sim_player({player_vertices.begin(), player_vertices.end()});
	sim_player.Translate({10, -50});
	sim_player.rb.angular_damping = 1.f;
	sim_player.rb.inv_mass = 1.f / 20.f;
	sim_player.rb.restitution = 0.5f;
	sim_player.rb.friction = 1.f;

	fz::Polygon floor({floor_vertices.begin(), floor_vertices.end()});
	floor.Translate({-50, 0});
	floor.rb.is_static = true;
	
	world.AddPolygon(sim_player);
	world.AddPolygon(floor);
}

void Sim::OnStart(Object* obj)
{
	Script::OnStart(obj);

	Time::SetFixedDeltaTime(0.015f);

#if defined(TOAD_EDITOR) || !defined(NDEBUG)
	txt_to_draw.clear();
#endif 

	Mouse::SetVisible(true);

	world = fz::World();
	vm = VM();

	// use the sim lib library 
	vm.RegisterLib(SimLib::GetSimLib());
	vm.RegisterLib(IO::GetIOLib());

	lmouse_released = false;
	lmouse_pressed = false;
	rmouse_released = false;
	rmouse_pressed = false;
	add_potential_spring = false;
	add_potential_square = false;
	run_vm = false;
	
	SetDefaultWorld(world);

	Input::AddMousePressCallback(OnMousePress);
	Input::AddMouseReleaseCallback(OnMouseRelease);
}

void Sim::OnUpdate(Object* obj)
{
	Script::OnUpdate(obj);

	Camera* cam = Camera::GetActiveCamera();
	if (cam)
		cam->SetPosition(world.polygons[0].rb.center);

	Vec2f world_mouse = Screen::ScreenToWorld(Mouse::GetPosition(), *Camera::GetActiveCamera());
	
	static Vec2f potential_square_pos;

	// spring previous spring polygon index
	static int i_prev = 0;
	static Vec2f rel_prev {};
	
	Timer timer(true);
	for (int i = 0; i < world.polygons.size(); i++)
	{
		fz::Polygon& curr_polygon = world.polygons[i];

		if (interact_with_mouse)
		{
			if (curr_polygon.ContainsPoint(world_mouse))
			{
				// do some 
				if (lmouse_pressed)
				{
					add_potential_spring = true;
					rel_prev = world_mouse - curr_polygon.rb.center;
					i_prev = i;
				}
				else if (add_potential_spring && lmouse_released)
				{
					// don't add spring to same rb 
					if (i_prev != i)
					{
						add_potential_spring = false;
						fz::Polygon& start_polygon = world.polygons[i_prev];
						fz::Polygon& end_polygon = curr_polygon;
						Vec2f end_rel = world_mouse - end_polygon.rb.center;
	
						fz::Spring& spr = world.AddSpring(start_polygon, end_polygon, rel_prev, end_rel);
						spr.stiffness = 1.f;
					}
				}
			}
			else
			{
				// add polygon using rmb 
	
				if (rmouse_pressed)
				{
					add_potential_square = true; 
					potential_square_pos = world_mouse;
				}
				else if (add_potential_square && rmouse_released)
				{
					add_potential_square = false; 
					Vec2f square_size = world_mouse - potential_square_pos;
					std::array<Vec2f, 6> square_vertices = fz::CreateSquare(square_size.x, square_size.y);
					fz::Polygon p({square_vertices.begin(), square_vertices.end()});
					p.Translate(potential_square_pos);
					world.AddPolygon(p);
				}
			}
		}
		
		if (show_velocities)
			DrawingCanvas::DrawArrow(curr_polygon.rb.center, curr_polygon.rb.velocity, 1.f);
		
		if (show_aabb)
			DrawingCanvas::DrawRect(curr_polygon.aabb.min, curr_polygon.aabb.max);
	}

	float drawing_canvas_time = timer.Elapsed<std::chrono::microseconds>() / 1000.f;
	DrawText("DrawingCanvas (No Springs): {}ms", drawing_canvas_time);
	DrawText("VM: {}ms", vm_time);
	DrawText("Sim: {}ms", sim_time);
	DrawText("FixedUpdate: {}ms", fixed_time);

	// int i = 0;
	for (const fz::Spring& spr : world.springs)
	{
		Vec2f a = spr.start_rb->center + spr.start_rel;
		Vec2f b = spr.end_rb->center + spr.end_rel;
		DrawingCanvas::DrawArrow(a, (b - a), 1.f);
	}

    for (const fz::Thruster& thr : world.thrusters)
	{
        const fz::Polygon& p = world.polygons[thr.attached_polygon];

		Vec2f a = p.rb.center + thr.attached_rel_pos;
		Vec2f b = a + thr.direction * thr.power;
		Vec2f b2 = a + thr.direction;
        Toad::Color c{255, 255, 0};
		DrawingCanvas::DrawArrow(a, (b2 - a) * 10, 1.f, c);
		DrawingCanvas::DrawArrow(a, (b - a) * 100, 1.f, c);
	}

	if (add_potential_spring && lmouse_released)
		add_potential_spring = false;
	if (add_potential_square && rmouse_released)
		add_potential_square = false;

	lmouse_pressed = false;
	lmouse_released = false;
	rmouse_pressed = false;
	rmouse_released = false;

	if (env_car_loaded)
		SimEnvironments::CarEnvironmentUpdate(env_car_gas * Time::GetDeltaTime());
}

void Sim::OnFixedUpdate(Object* obj)
{
	Timer total_timer(true);

	if (!pause_sim)
	{
		Timer timer(true);
		world.Update(Time::GetFixedDeltaTime());
		sim_time = timer.Elapsed<std::chrono::microseconds>() / 1000.f;
	}

	if (run_vm)
	{
		Timer timer(true);
		vm.Run(); 
		vm.instruction_pointer = 0;
		vm_time = timer.Elapsed<std::chrono::microseconds>() / 1000.f;
	}
	
	fixed_time = total_timer.Elapsed<std::chrono::microseconds>() / 1000.f;
}

void Sim::OnRender(Object* obj, sf::RenderTarget& target) 
{
	world.dc.DrawVertices(target, sf::PrimitiveType::TriangleStrip);
	world.dc.DrawBuffers(target);
}

void Sim::ExposeVars()
{
	Script::ExposeVars();
}

// #define TOAD_EDITOR // for intellisense in vscode 

#if defined(TOAD_EDITOR) || !defined(NDEBUG)

static void AddTestBuildMsg()
{
    ImDrawList* draw = ImGui::GetForegroundDrawList();
    ImVec2 pos = {ImGui::GetWindowSize()};
    const char* msg = "THIS IS A TEST BUILD - BUGS/CRASHES EXPECTED";
    ImVec2 text_size = ImGui::CalcTextSize(msg);
    pos.x /= 2.f;
    pos.x -= text_size.x / 2.f;
    pos.y = 50.f;

    draw->AddText(pos, IM_COL32(255, 0, 0, 255), msg);
}

void Sim::OnImGui(Object* obj,const UICtx&  ctx)
{
	UI_APPLY_CTX(ctx);

#ifndef TOAD_EDITOR
    AddTestBuildMsg();
#endif 

	static char source[1024];

    UI::FizzixMenu(*this, source);
    UI::TVMMenu(vm, run_vm, source);

	if (env_stress_test_loaded)
		SimEnvironments::StressTestImGui(*this);
}
#endif 
