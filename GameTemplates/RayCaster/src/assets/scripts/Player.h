#pragma once

#include "Engine/Engine.h"

// Handles the moving and the looking, also using the raycaster
class GAME_API Player : public Toad::Script
{
public:
	SCRIPT_CONSTRUCT(Player)

	void OnStart(Toad::Object* obj) override;
	void OnUpdate(Toad::Object* obj) override;
	void OnFixedUpdate(Toad::Object* obj) override;
	void ExposeVars() override;

	Vec2f vel{};
	float speed = 60.f;
	float mouse_look_sens = 0.8f;
	float player_size = 3.f;
};

