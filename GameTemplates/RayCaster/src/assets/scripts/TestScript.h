#pragma once

#include "engine/default_objects/Circle.h"

class GAME_API TestScript : public Toad::Script
{
public:
	SCRIPT_CONSTRUCT(TestScript)

	float start_direction_X = 1;
	float start_direction_Y = 1;
	float speed_mult = 1;

	float velx = 1;
	float vely = 1;

	void OnStart(Toad::Object* obj) override;
	void OnUpdate(Toad::Object* obj) override;
	void OnFixedUpdate(Toad::Object* obj) override;
	void ExposeVars() override;

#ifdef TOAD_EDITOR
	void OnEditorUI(Toad::Object* obj, ImGuiContext* ctx);
#endif
#if defined(TOAD_EDITOR) || !defined(NDEBUG)
	void OnImGui(Toad::Object* obj, ImGuiContext* ctx);
#endif 

private:
	Toad::Circle* circle = nullptr;

};
