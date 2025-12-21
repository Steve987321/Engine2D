#pragma once

#include "engine/FormatStr.h"

#ifdef USE_IMGUI
	inline std::unordered_map<const char*, std::string> txt_to_draw = {};

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
	class Sim;
}

// main script 
class GAME_API Sim : public Toad::Script
{
public:
	SCRIPT_CONSTRUCT(Sim)
    
	static fz::Sim& GetSim();
	static void SetDefaultScene(fz::Sim& sim);

	void OnStart(Toad::Object* obj) override;
	void OnUpdate(Toad::Object* obj) override;
	void OnFixedUpdate(Toad::Object* obj) override;
	void OnRender(Toad::Object* obj, sf::RenderTarget& target) override;
	void ExposeVars() override;

	bool pause_sim = false;
	inline static float d_y = 0;
	inline static float env_car_gas = 0.f;

#ifdef TOAD_EDITOR
	void OnEditorUI(Toad::Object* obj, const UICtx& ctx) override;
#endif
#ifdef USE_IMGUI
	void OnImGui(Toad::Object* obj, const UICtx& ctx) override;
#endif 
};
