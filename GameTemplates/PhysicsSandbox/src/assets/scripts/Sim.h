#pragma once

#include "engine/FormatStr.h"

#if defined(TOAD_EDITOR) || !defined(NDEBUG)
inline std::unordered_map<const char*, std::string> txt_to_draw = {};
inline std::vector<std::pair<Toad::Vec2f, Toad::Vec2f>> rect_to_draw = {};

template <typename ...T>	
inline void DrawText(std::string_view txt, T... args)
{
	txt_to_draw[txt.data()] = Toad::format_str(txt, args...);
}
#else 
template <typename ...T>	
inline void DrawText(std::string_view txt, T... args)
{}
#endif 

namespace fz
{
	class World;
}

/**
 * @brief Handles the everything.
 * Uses the drawing canvas for drawing
 */
class GAME_API Sim : public Toad::Script
{
public:
	SCRIPT_CONSTRUCT(Sim)
    
	static fz::World& GetWorld();
	static void SetDefaultWorld(fz::World& world);

	void OnStart(Toad::Object* obj) override;
	void OnUpdate(Toad::Object* obj) override;
	void OnFixedUpdate(Toad::Object* obj) override;
	void OnRender(Toad::Object* obj, sf::RenderTarget& target) override;
	void ExposeVars() override;

	bool pause_sim = false;
	bool env_car_loaded = false;
	bool env_stress_test_loaded = false;
	bool show_aabb = false; // show objects' aabb's 
	bool show_velocities = false; // show objects' velocity lines 
	bool interact_with_mouse = false; // use the mouse to add objects and attach springs

	inline static float env_car_gas = 0.f;

#if defined(TOAD_EDITOR) || !defined(NDEBUG)
	void OnImGui(Toad::Object* obj, const UICtx& ctx) override;
#endif 
};
