#pragma once

#include "Engine/Engine.h"

class GAME_API RayCaster : public Toad::Script
{
public:
	SCRIPT_CONSTRUCT(RayCaster)

	void OnStart(Toad::Object* obj) override;
	void OnUpdate(Toad::Object* obj) override;
	void ExposeVars() override;

	// fake pitch
	float pitch = 0.f;

	Toad::Sprite* floor_sprite = nullptr;

	constexpr static int fov = 90;
	constexpr static int fovhalf = fov / 2;
	constexpr static int fovhalfrad = fovhalf * (std::_Pi_val / 180.f);
	constexpr static int line_thickness = 1;
};

